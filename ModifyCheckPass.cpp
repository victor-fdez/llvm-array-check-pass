#include "ModifyCheckPass.h"

using namespace llvm;

char ModifyCheckPass::ID = 0;
static RegisterPass<ModifyCheckPass> C("modify-check", "Modify Array Bound Checks using very busy checks", false, false);

void ModifyCheckPass::generateMetadata(MDString* str, Value* variable, Value* n, Instruction* inst, Module* M)
{
	LLVMContext& context = M->getContext();
	std::vector<Value*> effect;
	effect.push_back(str);
	effect.push_back(variable);
	effect.push_back(n);
	MDNode* effects = MDNode::get(context, effect);
	inst->setMetadata("EFFECT", effects);
}

void ModifyCheckPass::modify(CallInst* callInst, RangeCheckSet* RCS, Module* M)
{
	RangeCheckExpression* expr = new RangeCheckExpression(callInst, M);
	
	MDString* incrementString = MDString::get(M->getContext(), "INCREMENT");
	MDString* decrementString = MDString::get(M->getContext(), "DECREMENT");

	std::vector<Value*> effect;

	for (std::vector<RangeCheckExpression>::iterator it = RCS->checkSet->begin(); it != RCS->checkSet->end(); ++it)
	{
		RangeCheckExpression* expr2 = &(*it);

		// Each side of RCE should constain: 1. constants only or, 2. Single same variable with different offsets
		if (expr2->subsumes(expr) && !(expr->subsumes(expr2)))
		{
			errs() << "Replacing "; expr->print();
			errs() << " with "; expr2->println();
			
			if (dyn_cast<ConstantInt>(expr2->left) && dyn_cast<ConstantInt>(expr->left)) // LHS contains a constant
			{
				Value* arguement = expr2->left;

				if ((expr2->left->getType()) != Type::getInt64Ty(this->M->getContext()))
				{
						if (expr2->left->getType()->isPointerTy())
							arguement = CastInst::CreatePointerCast(arguement, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
						else
							arguement = CastInst::CreateIntegerCast(arguement, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
//				errs() << "func arg0: " << *(expr2->left) << "\n";
//				errs() << "func meta0: " << *(expr2->left) << "\n";
				callInst->setArgOperand(0, arguement);
				effect.push_back(arguement);
			}
			else if (dyn_cast<AllocaInst>(expr2->left) || dyn_cast<GlobalVariable>(expr2->left)) // LHS contains a named variable
			{

				Instruction* loadInst = new LoadInst(expr2->left, "MODIFY_CHECK", callInst);
				
				if (expr2->left->getType() != Type::getInt64Ty(this->M->getContext()))
				{
						loadInst = CastInst::CreateIntegerCast(loadInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
				
//				errs() << "func arg0: " << *loadInst << "\n";
//				errs() << "func meta0: " << *(expr2->left) << "\n";
				
				callInst->setArgOperand(0, loadInst);
				effect.push_back(expr2->left);
			}
			else if (dyn_cast<Instruction>(expr2->left)) // LHS contains a temporary variable
			{
				MDNode* metadata2 = dyn_cast<Instruction>(expr2->left)->getMetadata("EFFECT");
				
				if (metadata2 == NULL)
				{
					errs() << "ERROR: THIS CASE SHOULD NOT HOLD!! (MY ASSUMPTION IN MODIFYPASS)\n";
				}


				Value* originVar = metadata2->getOperand(1);
				Value* addVal = dyn_cast<ConstantInt>(metadata2->getOperand(2));
				Value* loadInst = new LoadInst(originVar, "MODIFY_CHECK", callInst);
				
				if ((loadInst->getType()) != Type::getInt64Ty(this->M->getContext()))
				{
						loadInst = CastInst::CreateIntegerCast(loadInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
				
				if ((addVal->getType()) != Type::getInt64Ty(this->M->getContext()))
				{
					if (addVal->getType()->isPointerTy())
						addVal = CastInst::CreatePointerCast(addVal, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
					else
						addVal = CastInst::CreateIntegerCast(addVal, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}

				Instruction* addInst = llvm::BinaryOperator::Create(Instruction::Add, loadInst, addVal, "MODIFY_CHECK", callInst);
				generateMetadata(incrementString, originVar, addVal, addInst, M);
				
				if (addInst->getType() != Type::getInt64Ty(this->M->getContext()))
				{
					if (addInst->getType()->isPointerTy())
						addInst = CastInst::CreatePointerCast(addInst, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
					else
						addInst = CastInst::CreateIntegerCast(addInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}

//				errs() << "func arg0: " << *addInst << "\n";
//				errs() << "func meta0: " << *addInst << "\n";
				
				callInst->setArgOperand(0, addInst);
				effect.push_back(addInst);
			}	
			else
			{
					errs() << "ERROR!!! SUBSUME ASSUMPTION VIOLATED IN MODIFY PASS!!!\n";
			}	

			
			if (dyn_cast<ConstantInt>(expr2->right) && dyn_cast<ConstantInt>(expr->right)) // RHS contains a constant
			{
//				errs() << "func arg1: " << *(expr2->right) << "\n";
//				errs() << "func meta1: " << *(expr2->right) << "\n";
				Value* arguement = expr2->right;
				if ((expr2->right->getType()) != Type::getInt64Ty(this->M->getContext()))
				{
					if (expr2->right->getType()->isPointerTy())
						arguement = CastInst::CreatePointerCast(arguement, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
					else
						arguement = CastInst::CreateIntegerCast(arguement, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}

				callInst->setArgOperand(1, arguement);
				effect.push_back(arguement);
			
			}
			else if (dyn_cast<AllocaInst>(expr2->right) || dyn_cast<GlobalVariable>(expr2->right)) // RHS contains a named variable
			{
				Instruction* loadInst = new LoadInst(expr2->right, "MODIFY_CHECK", callInst);
				errs() << "RHS: " << *(expr2->right) << "\n";
				errs() << "Load: " << *loadInst << "\n";
				if (expr2->right->getType() != Type::getInt64Ty(this->M->getContext()))
				{
						loadInst = CastInst::CreateIntegerCast(loadInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
//				errs() << "func arg1: " << *loadInst << "\n";
//				errs() << "func meta1: " << *(expr2->right) << "\n";
				callInst->setArgOperand(1, loadInst);
				effect.push_back(expr2->right);
			}
			else if (dyn_cast<Instruction>(expr2->right)) // LHS contains a temporary variable
			{
				MDNode* metadata2 = dyn_cast<Instruction>(expr2->right)->getMetadata("EFFECT");
				
				if (metadata2 == NULL)
				{
					errs() << "ERROR: THIS CASE SHOULD NOT HOLD!! (ASSUMPTION IN MODIFYPASS)\n";
				}

				Value* originVar = metadata2->getOperand(1);
				Value* addVal = dyn_cast<ConstantInt>(metadata2->getOperand(2));
				Value* loadInst = new LoadInst(originVar, "MODIFY_CHECK", callInst);

				if (loadInst->getType() != Type::getInt64Ty(this->M->getContext()))
				{
						loadInst = CastInst::CreateIntegerCast(loadInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
				
				if (addVal->getType() != Type::getInt64Ty(this->M->getContext()))
				{
					if (addVal->getType()->isPointerTy())
						addVal = CastInst::CreatePointerCast(addVal, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
					else
						addVal = CastInst::CreateIntegerCast(addVal, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}

				Instruction* addInst = llvm::BinaryOperator::Create(Instruction::Add, loadInst, addVal, "MODIFY_CHECK", callInst);
				generateMetadata(decrementString, originVar, addVal, addInst, M);
				
				if (addInst->getType() != Type::getInt64Ty(this->M->getContext()))
				{
					if (addInst->getType()->isPointerTy())
						addInst = CastInst::CreatePointerCast(addInst, Type::getInt64PtrTy(this->M->getContext()), "MODIFY_CHECK", callInst);
					else
						addInst = CastInst::CreateIntegerCast(addInst, Type::getInt64Ty(this->M->getContext()), true, "MODIFY_CHECK", callInst);
				}
				
//				errs() << "func arg1: " << *addInst << "\n";
//				errs() << "func meta1: " << *addInst << "\n";

				callInst->setArgOperand(1, addInst);
				effect.push_back(addInst);
			}	
			else
			{
					errs() << "ERROR!!! SUBSUME ASSUMPTION VIOLATED IN MODIFY PASS!!!\n";
			}

			MDNode* effects = MDNode::get(this->M->getContext(), effect);
			callInst->setMetadata("VarName", effects);
			
			//errs() << "modified: " << *callInst << "\n";
			return;	
		}
	}
}

bool ModifyCheckPass::runOnModule(Module& M)
{
	this->M = &M;
	this->veryBusyAnalysis = &getAnalysis<VeryBusyAnalysisPass>();
	this->veryBusyMap = this->veryBusyAnalysis->I_VB_IN;

	errs() << "\n#########################################\n";
	errs() << "MODIFYCHECK PASS\n";
	errs() << "#########################################\n";

	for (Module::iterator i = M.begin(), e = M.end(); i != e; ++i)
	{
		Function* func = &(*i);
		runOnFunction(&(*func));
	}
	
//	errs() << "\n#########################################\n";
//	errs() << "DONE\n";
//	errs() << "#########################################\n";

	return false;
}

bool ModifyCheckPass::runOnFunction(Function* F)
{
	for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i)
	{
		runOnBasicBlock(i);
	}
	return true;
}

bool ModifyCheckPass::runOnBasicBlock(BasicBlock* BB)
{
	for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i) 
	{
		Instruction *inst = &*i;
		 		
		if (CallInst *CI = dyn_cast<CallInst>(inst)) 
		{
			if (CI->getCalledFunction() == NULL)
				continue;
			StringRef funcName = CI->getCalledFunction()->getName();
			
			if (funcName.equals("checkLessThan"))
			{
				//errs() << "-------------------------------------------------------------------\n";
				//errs() << "inst: " << *CI << "\n";
				RangeCheckSet* current = (*veryBusyMap)[inst];
				//errs() << "Very Busy Expressions: "; current->println();
				modify(CI, current, this->M);
				//errs() << "-------------------------------------------------------------------\n";
			}
		}
	}
    return true;
}
