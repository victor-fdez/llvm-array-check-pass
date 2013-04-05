
#ifndef __TEST_PASS_H__
#define __TEST_PASS_H__

#include "EffectGenPass.h"
#include "llvm/User.h"
#include "llvm/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/Operator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/InstrTypes.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Analysis/Dominators.h"
#include "ArrayBoundsCheckPass.h"
//#include "RunTimeBoundsChecking.h"

namespace llvm 
{
	struct testPass : public ModulePass 
	{
		public: 
			static char ID;
			testPass() : ModulePass(ID) {}
			virtual bool runOnModule(Module& M);
			virtual void getAnalysisUsage(AnalysisUsage &AU) const {
				AU.addRequired<EffectGenPass>();
			}
		private:
			bool runOnFunction(Function* func);
			bool runOnBasicBlock(BasicBlock* BB);
			Module* M;
	};
}

#endif /* __LOCAL_OPTIMIZATIONS_ON_ARRAY_CHECKS_H__ */