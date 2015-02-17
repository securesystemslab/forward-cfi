//===-- X86ReloadIP.cpp - Add Reload IP instructions            -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the pass that finds instructions that add reload IPs
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "X86MachineFunctionInfo.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/CodeGen/JumpInstrTables.h"
#include "llvm/Analysis/JumpInstrTableInfo.h"

using namespace llvm;

#define DEBUG_TYPE "x86-reload-IPs"

namespace {
    
    class ReloadIPPass : public MachineFunctionPass {
        
        enum RegUsageState {
            RU_NotUsed, RU_Write, RU_Read
        };
        static char ID;
        
        /// \brief Loop over all of the basic blocks,
        /// adding reload IP instructions.
        void processBasicBlock(MachineFunction &MF, MachineFunction::iterator MFI);
        
        const char *getPassName() const override {
            return "X86 Reload IP";
        }
        
        MachineInstr *addReloadIPInstr(MachineFunction::iterator &MFI,
                                       MachineBasicBlock::iterator &MBBI) const;
        
    public:
        
        ReloadIPPass() : MachineFunctionPass(ID) {
        }
        
        /// \brief Loop over all of the basic blocks,
        /// adding reload IP instructions.
        bool runOnMachineFunction(MachineFunction &MF) override;
        
    private:
        MachineFunction *MF;
        const TargetMachine *TM;
        const X86InstrInfo *TII; // Machine instruction info.
    };
    char ReloadIPPass::ID = 0;
}

MachineInstr *
ReloadIPPass::addReloadIPInstr(MachineFunction::iterator &MFI,
                               MachineBasicBlock::iterator &MBBI) const {
    MachineInstr *MI = MBBI;
    StringRef JTPrefix(JumpInstrTables::jump_func_prefix);
    switch (MI->getOpcode()) {
        case X86::MOV32rm:
            if (MI->getNumOperands() > 4 && MI->getOperand(4).isGlobal() &&
                MI->getOperand(4).getGlobal()->getName().startswith(JTPrefix)) {
                if ((TM->Options.CFIType == CFIntegrity::Add && MI->getNextNode()->getOpcode() == X86::AND32rr)
                    || (TM->Options.CFIType == CFIntegrity::Sub && MI->getNextNode()->getOpcode() == X86::SUB32rr)
                    || (TM->Options.CFIType == CFIntegrity::Ror && MI->getNextNode()->getOpcode() == X86::SUB32rr)
                    ) {
                    //                    MI->dump(); // For debugging
                    DEBUG(dbgs() << TM->getSubtargetImpl()->getInstrInfo()->getName(MI->getOpcode()) << ": "
                          << " Op" << ": "
                          << MI->getOperand(4).getGlobal()->getName().str() << "\n";);
                    
                    return MI;
                } else
                    MI->getParent()->dump(); // For debugging
            }
            break;
            // Start adding reload ip instruction after the first call **experimental**
            //    case X86::CALL32m:
            //    case X86::CALL32r:
            //    case X86::CALLpcrel32:
            // TODO: add boolean variable to trigger the first call
            //      break;
            
    }
    
    return nullptr;
}

FunctionPass *llvm::createX86ReloadIPs() {
    return new ReloadIPPass();
}

bool ReloadIPPass::runOnMachineFunction(MachineFunction &Func) {
    MF = &Func;
    TM = &Func.getTarget();
    TII = static_cast<const X86InstrInfo *> (TM->getSubtargetImpl()->getInstrInfo());
    
    DEBUG(dbgs() << "Processing Function: " << Func.getName().str()
          << "  Func#" << Func.getFunctionNumber() << "\n";);
    
    //skip __cxx_global_var_init function **experimental**
    //  if(Func.getFunctionNumber() == 0)
    //      return true;
    DEBUG(dbgs() << "Start X86ReloadIPs for " << Func.getName().str()
          << "  Func#" << Func.getFunctionNumber() << "\n";);
    
    // Process all basic blocks.
    for (MachineFunction::iterator I = Func.begin(), E = Func.end(); I != E; ++I)
        processBasicBlock(Func, I);
    
    DEBUG(dbgs() << "End X86ReloadIPs for " << Func.getName().str()
          << "  Func#" << Func.getFunctionNumber() << "\n";);
    
    return true;
}

void ReloadIPPass::processBasicBlock(MachineFunction &MF,
                                     MachineFunction::iterator MFI) {
    
    SmallVector<MachineInstr*, 4> fcfiChecks;
    for (MachineBasicBlock::iterator I = MFI->begin(); I != MFI->end(); ++I) {
        MachineInstr *fcfiProcess = addReloadIPInstr(MFI, I);
        if (fcfiProcess)
            fcfiChecks.push_back(fcfiProcess);
    }
    DEBUG(dbgs() << "Number of insertions: " << fcfiChecks.size() << "\n";);
    for (MachineInstr* &MI : fcfiChecks) {
        unsigned VReg = MF.getRegInfo().createVirtualRegister(&X86::GR32_NOSPRegClass);
        unsigned VReg2 = MF.getRegInfo().createVirtualRegister(&X86::GR32_NOSPRegClass);
        
        //Dummy instruction to trick LiveRange of Regs
        BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(X86::DUMMYFCFI), VReg);
        
        //Add machine instructions
        BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(X86::RELOADIP), VReg2)
        .addReg(VReg).addExternalSymbol("_GLOBAL_OFFSET_TABLE_",
                                        X86II::MO_GOT_ABSOLUTE_ADDRESS);
        
        
        //Update jump instruction table to use just produced IP reg
        MI->getOperand(1).setReg(VReg2);
    }
}

