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
    if (F.isDeclaration())
      return PreservedAnalyses::all();

    errs() << "Processing function: " << F.getName() << "\n";

    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);

    bool Modified = false;
    
    // 我們使用後序遍歷 (Post-Order) 來處理迴圈
    // 這樣可以先處理內層迴圈，再處理外層迴圈
    for (auto *L : LI) {
      for (auto *SubL : L->getLoopsInPreorder()) {
         Modified |= processLoop(SubL, LI, DT);
      }
      Modified |= processLoop(L, LI, DT);
    }

    if (Modified) {
      return PreservedAnalyses::none();
    }
    return PreservedAnalyses::all();
  }

  // 核心邏輯：處理單個迴圈
  bool processLoop(Loop *L, LoopInfo &LI, DominatorTree &DT) {
    bool Changed = false;
    
    // 1. 確保迴圈有 Preheader (我們可以把東西搬過去的地方)
    BasicBlock *Preheader = L->getLoopPreheader();
    if (!Preheader) {
      errs() << "  Loop does not have a single preheader. Skipping.\n";
      return false;
    }

    errs() << "  Checking Loop at depth " << L->getLoopDepth() << "\n";

    // 2. 收集所有想要外提的指令
    // 我們不能邊遍歷邊修改，所以先存起來
    std::vector<Instruction*> InstructionsToHoist;

    for (BasicBlock *BB : L->blocks()) {
      // 簡單起見，我們只處理 Loop Header 或是必定執行的 block (這裡簡化處理所有 block)
      // 在嚴格的 LICM 中需要確認該 block 是否支配所有出口，但作業示範我們先檢查指令本身
      
      for (Instruction &I : *BB) {
        // 檢查指令是否適合外提
        if (canHoist(I, L)) {
           InstructionsToHoist.push_back(&I);
        }
      }
    }

    // 3. 執行搬移 (Hoisting)
    for (Instruction *I : InstructionsToHoist) {
      errs() << "    Hoisting instruction: " << I->getOpcodeName() << "\n";
      
      // 將指令搬移到 Preheader 的終止指令(Branch)之前
      I->moveBefore(Preheader->getTerminator());
      Changed = true;
    }

    if (Changed) 
        errs() << "    Successfully hoisted " << InstructionsToHoist.size() << " instructions.\n";

    return Changed;
  }

  // 檢查指令是否為迴圈不變量 (Loop Invariant) 且安全可移動
  bool canHoist(Instruction &I, Loop *L) {
    // 條件 1: 必須是「安全」的指令
    // 我們不移動 Memory Load/Store (除非有 Alias Analysis)，不移動 Call，不移動 Branch
    // 只移動簡單的算術運算 (Binary Operators)
    if (!I.isBinaryOp() && !I.isShift() && !I.isBitwiseLogicOp() && !isa<CastInst>(I)) {
      return false;
    }

    // 條件 2: 不能有 Side Effects (例如除以 0)
    // isSafeToSpeculativelyExecute 幫助我們檢查這個指令是否會造成 Crash
    if (!isSafeToSpeculativelyExecute(&I)) {
      return false;
    }

    // 條件 3: 操作數 (Operands) 必須是迴圈不變量 (Loop Invariant)
    // 也就是說，所有輸入參數要嘛是常數，要嘛是在迴圈外部定義的
    for (Use &Op : I.operands()) {
      Value *V = Op.get();
      
      // 如果操作數是常數，那就是不變的
      if (isa<Constant>(V)) continue;
      
      // 如果操作數是指令，檢查它是否定義在迴圈內
      if (Instruction *OpInst = dyn_cast<Instruction>(V)) {
        if (L->contains(OpInst)) {
          // 定義在迴圈內，那這個指令就不是不變量 (除非我們已經遞迴地證明它是，但這裡簡化)
          return false;
        }
      }
      
      // 如果是參數 (Argument)，那一定是在外部定義的，安全
    }

    return true;
  }
};

} // end anonymous namespace

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