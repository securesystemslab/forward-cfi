//===-- JumpInstrTables.h: Jump-Instruction Tables --------------*- C++ -*-===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief An implementation of tables consisting of jump instructions
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_JUMPINSTRTABLES_H
#define LLVM_CODEGEN_JUMPINSTRTABLES_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetOptions.h"

namespace llvm {
class Constant;
class Function;
class FunctionType;
class JumpInstrTableInfo;
class Module;

/// A class to manage a set of jump tables indexed on function type. It looks at
/// each function in the module to find all the functions that have the
/// jumptable attribute set. For each such function, it creates a new
/// jump-instruction-table function and stores the mapping in the ImmutablePass
/// JumpInstrTableInfo.
///
/// These special functions get lowered in AsmPrinter to assembly of the form:
/// \verbatim
///   .globl f
///   .type f,@function
///   .align 8,0x90
/// f:
///   jmp f_orig@PLT
/// \endverbatim
///
/// Support for an architecture depends on three functions in TargetInstrInfo:
/// getUnconditionalBranch, getTrap, and getJumpInstrTableEntryBound. AsmPrinter
/// uses these to generate the appropriate instructions for the jump statement
/// (an unconditional branch) and for padding to make the table have a size that
/// is a power of two. This padding uses a trap instruction to ensure that calls
/// to this area halt the program. The default implementations of these
/// functions call llvm_unreachable, except for getJumpInstrTableEntryBound,
/// which returns 0 by default.
class JumpInstrTables : public ModulePass {
public:
  static char ID;
  static const char jump_func_prefix[];
  static const char jump_section_prefix[];
  JumpInstrTables();
  JumpInstrTables(JumpTable::JumpTableType JTT);
  virtual ~JumpInstrTables();
  bool runOnModule(Module &M) override;
  const char *getPassName() const override { return "Jump-Instruction Tables"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  /// Creates a jump-instruction table function for the Target and adds it to
  /// the tables.
  Function *insertEntry(Module &M, Function *Target);

  /// Checks to see if there is already a table for the given FunctionType.
  bool hasTable(FunctionType *FunTy);

  /// Maps the function into a subset of function types, depending on the
  /// jump-instruction table style selected from JumpTableTypes in
  /// JumpInstrTables.cpp. The choice of mapping determines the number of
  /// jump-instruction tables generated by this pass. E.g., the simplest mapping
  /// converts every function type into void f(); so, all functions end up in a
  /// single table.
  static FunctionType *transformType(JumpTable::JumpTableType JTT,
                                     FunctionType *FunTy);
private:
  /// The metadata used while a jump table is being built
  struct TableMeta {
    /// The number of this table
    unsigned TableNum;

    /// The current number of jump entries in the table.
    unsigned Count;
  };

  typedef DenseMap<FunctionType *, struct TableMeta> JumpMap;

  /// The current state of functions and jump entries in the table(s).
  JumpMap Metadata;

  /// The ImmutablePass that stores information about the generated tables.
  JumpInstrTableInfo *JITI;

  /// The total number of tables.
  unsigned TableCount;

  /// The type of tables to build.
  JumpTable::JumpTableType JTType;
};

/// Creates a JumpInstrTables pass for the given type of jump table.
ModulePass *createJumpInstrTablesPass(JumpTable::JumpTableType JTT);
}

#endif /* LLVM_CODEGEN_JUMPINSTRTABLES_H */
