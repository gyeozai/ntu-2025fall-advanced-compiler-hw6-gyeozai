#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/ValueTracking.h" 
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace llvm;

namespace {

struct LoopLICMPass : public PassInfoMixin<LoopLICMPass> {
  
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    // Step 1: Skip external declarations; only process defined functions
    if (F.isDeclaration())
      return PreservedAnalyses::all();

    errs() << "Processing function: " << F.getName() << "\n";

    // Step 2: Retrieve necessary analysis results (Loop Information and Dominator Tree)
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);

    bool Modified = false;
    
    // Step 3: Iterate through loops
    // We process inner loops first (Post-Order style) to maximize hoisting efficiency
    for (auto *L : LI) {
      for (auto *SubL : L->getLoopsInPreorder()) {
         Modified |= processLoop(SubL, LI, DT);
      }
      Modified |= processLoop(L, LI, DT);
    }

    // Step 4: Signal the Pass Manager if the IR was modified
    if (Modified) {
      return PreservedAnalyses::none();
    }
    return PreservedAnalyses::all();
  }

  // Core Logic: Processing a single loop
  bool processLoop(Loop *L, LoopInfo &LI, DominatorTree &DT) {
    bool Changed = false;
    
    // Step 1: Ensure the loop has a Preheader
    // This is the destination block where hoisted instructions will be moved
    BasicBlock *Preheader = L->getLoopPreheader();
    if (!Preheader) {
      errs() << "  Loop does not have a single preheader. Skipping.\n";
      return false;
    }

    errs() << "  Checking Loop at depth " << L->getLoopDepth() << "\n";

    // Step 2: Collect instructions that are eligible for hoisting
    // We store them in a vector first to avoid modifying the block while iterating
    std::vector<Instruction*> InstructionsToHoist;

    for (BasicBlock *BB : L->blocks()) {
      for (Instruction &I : *BB) {
        // Step 3: Check if the instruction is a loop invariant
        if (canHoist(I, L)) {
           InstructionsToHoist.push_back(&I);
        }
      }
    }

    // Step 4: Perform the Hoisting (Movement)
    for (Instruction *I : InstructionsToHoist) {
      errs() << "    Hoisting instruction: " << I->getOpcodeName() << "\n";
      
      // Move instruction to the end of the preheader (just before the branch)
      I->moveBefore(Preheader->getTerminator());
      Changed = true;
    }

    if (Changed) 
        errs() << "    Successfully hoisted " << InstructionsToHoist.size() << " instructions.\n";

    return Changed;
  }

  // Safety and Invariance Check
  bool canHoist(Instruction &I, Loop *L) {
    // Step 1: Filter by instruction type
    // We only move simple arithmetic/logic; avoid Memory (Load/Store), Calls, or Branches
    if (!I.isBinaryOp() && !I.isShift() && !I.isBitwiseLogicOp() && !isa<CastInst>(I)) {
      return false;
    }

    // Step 2: Check for potential side effects
    // Ensure the instruction won't crash (e.g., division by zero) if executed speculatively
    if (!isSafeToSpeculativelyExecute(&I)) {
      return false;
    }

    // Step 3: Verify that all operands are Loop Invariants
    // All inputs must be constants, function arguments, or defined outside this loop
    for (Use &Op : I.operands()) {
      Value *V = Op.get();
      
      if (isa<Constant>(V)) continue;
      
      if (Instruction *OpInst = dyn_cast<Instruction>(V)) {
        if (L->contains(OpInst)) {
          // If the operand is defined inside the loop, it's not invariant
          return false;
        }
      }
    }

    return true;
  }
};

} // end anonymous namespace

// Step 5: Register the pass as a plugin for the LLVM 'opt' tool
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "SimpleLoopLICM", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "simple-licm") {
            FPM.addPass(LoopLICMPass());
            return true;
          }
          return false;
        }
      );
    }
  };
}