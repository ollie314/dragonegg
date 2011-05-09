//=---- Internals.h - Interface between the backend components ----*- C++ -*-=//
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011  Chris Lattner,
// Duncan Sands et al.
//
// This file is part of DragonEgg.
//
// DragonEgg is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later version.
//
// DragonEgg is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// DragonEgg; see the file COPYING.  If not, write to the Free Software
// Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
//
//===----------------------------------------------------------------------===//
// This file declares the internal interfaces shared among the dragonegg files.
//===----------------------------------------------------------------------===//

#ifndef DRAGONEGG_INTERNALS_H
#define DRAGONEGG_INTERNALS_H

// LLVM headers
#include "llvm/Intrinsics.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/TargetFolder.h"

struct basic_block_def;
union gimple_statement_d;
union tree_node;

extern "C" {
extern void debug_gimple_stmt(union gimple_statement_d *);
extern void debug_tree(union tree_node *);
}

namespace llvm {
  class Module;
  class GlobalVariable;
  class Function;
  class GlobalValue;
  class BasicBlock;
  class Instruction;
  class AllocaInst;
  class BranchInst;
  class Value;
  class Constant;
  class ConstantInt;
  class Type;
  class FunctionType;
  class TargetMachine;
  class TargetData;
  class DebugInfo;
  template<typename> class AssertingVH;
  template<typename> class TrackingVH;
}
using namespace llvm;

typedef IRBuilder<true, TargetFolder> LLVMBuilder;

// Global state.

/// TheModule - This is the current global module that we are compiling into.
///
extern llvm::Module *TheModule;

/// TheDebugInfo - This object is responsible for gather all debug information.
/// If it's value is NULL then no debug information should be gathered.
extern llvm::DebugInfo *TheDebugInfo;

/// TheTarget - The current target being compiled for.
///
extern llvm::TargetMachine *TheTarget;

/// TheFolder - The constant folder to use.
extern TargetFolder *TheFolder;

/// getTargetData - Return the current TargetData object from TheTarget.
const TargetData &getTargetData();

/// flag_default_initialize_globals - Whether global variables with no explicit
/// initial value should be zero initialized.
extern bool flag_default_initialize_globals;

/// flag_odr - Whether the language being compiled obeys the One Definition Rule
/// (i.e. if the same function is defined in multiple compilation units, all the
/// definitions are equivalent).
extern bool flag_odr;

/// flag_vararg_requires_arguments - Do not consider functions with no arguments
/// to take a variable number of arguments (...).  If set then a function like
/// "T foo() {}" will be treated like "T foo(void) {}" and not "T foo(...) {}".
extern bool flag_vararg_requires_arguments;

/// flag_force_vararg_prototypes - Force prototypes to take a variable number of
/// arguments (...).  This is helpful if the language front-end sometimes emits
/// calls where the call arguments do not match the callee function declaration.
extern bool flag_force_vararg_prototypes;

/// AttributeUsedGlobals - The list of globals that are marked attribute(used).
extern SmallSetVector<Constant *,32> AttributeUsedGlobals;

extern Constant* ConvertMetadataStringToGV(const char *str);

/// DieAbjectly - An unrecoverable fatal error occurred - throw in the towel,
/// give up the ghost, quit miserably.
inline void LLVM_ATTRIBUTE_NORETURN DieAbjectly(const char *Message) {
  llvm_unreachable(Message);
}
inline void LLVM_ATTRIBUTE_NORETURN DieAbjectly(const char *Message,
                                                union gimple_statement_d *stmt){
  if (stmt) debug_gimple_stmt(stmt);
  DieAbjectly(Message);
}
inline void LLVM_ATTRIBUTE_NORETURN DieAbjectly(const char *Message,
                                                union tree_node *exp) {
  if (exp) debug_tree(exp);
  DieAbjectly(Message);
}

/// AddAnnotateAttrsToGlobal - Adds decls that have a
/// annotate attribute to a vector to be emitted later.
extern void AddAnnotateAttrsToGlobal(GlobalValue *GV, tree_node *decl);

// Mapping between GCC declarations and LLVM values.  The GCC declaration must
// satisfy HAS_RTL_P.

/// DECL_LLVM - Returns the LLVM declaration of a global variable or function.
extern Value *make_decl_llvm(tree_node *);
#define DECL_LLVM(NODE) make_decl_llvm(NODE)

/// SET_DECL_LLVM - Set the DECL_LLVM for NODE to LLVM.
extern Value *set_decl_llvm(tree_node *, Value *);
#define SET_DECL_LLVM(NODE, LLVM) set_decl_llvm(NODE, LLVM)

/// DECL_LLVM_IF_SET - The DECL_LLVM for NODE, if it is set, or NULL, if it is
/// not set.
extern Value *get_decl_llvm(tree_node *);
#define DECL_LLVM_IF_SET(NODE) (HAS_RTL_P(NODE) ? get_decl_llvm(NODE) : NULL)

/// DECL_LLVM_SET_P - Returns nonzero if the DECL_LLVM for NODE has already
/// been set.
#define DECL_LLVM_SET_P(NODE) (DECL_LLVM_IF_SET(NODE) != NULL)

/// DEFINITION_LLVM - Ensures that the body or initial value of the given GCC
/// global will be output, and returns a declaration for it.
Value *make_definition_llvm(tree_node *decl);
#define DEFINITION_LLVM(NODE) make_definition_llvm(NODE)

// Mapping between GCC declarations and non-negative integers.  The GCC
// declaration must not satisfy HAS_RTL_P.

/// set_decl_index - Associate a non-negative number with the given GCC
/// declaration.
int set_decl_index(tree_node *, int);

/// get_decl_index - Get the non-negative number associated with the given GCC
/// declaration.  Returns a negative value if no such association has been made.
int get_decl_index(tree_node *);

void changeLLVMConstant(Constant *Old, Constant *New);
void register_ctor_dtor(Function *, int, bool);
void readLLVMTypesStringTable();
void writeLLVMTypesStringTable();
void readLLVMValues();
void writeLLVMValues();
void clearTargetBuiltinCache();
const char *extractRegisterName(tree_node *);
void handleVisibility(tree_node *decl, GlobalValue *GV);
Twine getLLVMAssemblerName(tree_node *);

struct StructTypeConversionInfo;

/// Return true if and only if field no. N from struct type T is a padding
/// element added to match llvm struct type size and gcc struct type size.
bool isPaddingElement(tree_node*, unsigned N);

/// TypeConverter - Implement the converter from GCC types to LLVM types.
///
class TypeConverter {
  /// ConvertingStruct - If we are converting a RECORD or UNION to an LLVM type
  /// we set this flag to true.
  bool ConvertingStruct;

  /// PointersToReresolve - When ConvertingStruct is true, we handling of
  /// POINTER_TYPE and REFERENCE_TYPE is changed to return
  /// opaque*'s instead of recursively calling ConvertType.  When this happens,
  /// we add the POINTER_TYPE to this list.
  ///
  std::vector<tree_node*> PointersToReresolve;
public:
  TypeConverter() : ConvertingStruct(false) {}

  /// ConvertType - Returns the LLVM type to use for memory that holds a value
  /// of the given GCC type (getRegType should be used for values in registers).
  const Type *ConvertType(tree_node *type);

  /// GCCTypeOverlapsWithLLVMTypePadding - Return true if the specified GCC type
  /// has any data that overlaps with structure padding in the specified LLVM
  /// type.
  static bool GCCTypeOverlapsWithLLVMTypePadding(tree_node *t, const Type *Ty);


  /// ConvertFunctionType - Convert the specified FUNCTION_TYPE or METHOD_TYPE
  /// tree to an LLVM type.  This does the same thing that ConvertType does, but
  /// it also returns the function's LLVM calling convention and attributes.
  const FunctionType *ConvertFunctionType(tree_node *type,
                                          tree_node *decl,
                                          tree_node *static_chain,
                                          CallingConv::ID &CallingConv,
                                          AttrListPtr &PAL);

  /// ConvertArgListToFnType - Given a DECL_ARGUMENTS list on an GCC tree,
  /// return the LLVM type corresponding to the function.  This is useful for
  /// turning "T foo(...)" functions into "T foo(void)" functions.
  const FunctionType *ConvertArgListToFnType(tree_node *type,
                                             tree_node *arglist,
                                             tree_node *static_chain,
                                             CallingConv::ID &CallingConv,
                                             AttrListPtr &PAL);

private:
  const Type *ConvertRECORD(tree_node *type);
  bool DecodeStructFields(tree_node *Field, StructTypeConversionInfo &Info);
  void DecodeStructBitField(tree_node *Field, StructTypeConversionInfo &Info);
  void SelectUnionMember(tree_node *type, StructTypeConversionInfo &Info);
};

extern TypeConverter *TheTypeConverter;

/// getRegType - Returns the LLVM type to use for registers that hold a value
/// of the scalar GCC type 'type'.  All of the EmitReg* routines use this to
/// determine the LLVM type to return.
const Type *getRegType(tree_node *type);

/// ConvertType - Returns the LLVM type to use for memory that holds a value
/// of the given GCC type (getRegType should be used for values in registers).
inline const Type *ConvertType(tree_node *type) {
  return TheTypeConverter->ConvertType(type);
}

/// getDefaultValue - Return the default value to use for a constant or global
/// that has no value specified.  For example in C like languages such variables
/// are initialized to zero, while in Ada they hold an undefined value.
inline Constant *getDefaultValue(const Type *Ty) {
  return flag_default_initialize_globals ?
    Constant::getNullValue(Ty) : UndefValue::get(Ty);
}

/// GetUnitType - Returns an integer one address unit wide if 'NumUnits' is 1;
/// otherwise returns an array of such integers with 'NumUnits' elements.  For
/// example, on a machine which has 16 bit bytes returns an i16 or an array of
/// i16.
extern const Type *GetUnitType(LLVMContext &C, unsigned NumUnits = 1);

/// GetUnitPointerType - Returns an LLVM pointer type which points to memory one
/// address unit wide.  For example, on a machine which has 16 bit bytes returns
/// an i16*.
extern const Type *GetUnitPointerType(LLVMContext &C, unsigned AddrSpace = 0);

/// GetFieldIndex - Return the index of the field in the given LLVM type that
/// corresponds to the GCC field declaration 'decl'.  This means that the LLVM
/// and GCC fields start in the same byte (if 'decl' is a bitfield, this means
/// that its first bit is within the byte the LLVM field starts at).  Returns
/// INT_MAX if there is no such LLVM field.
int GetFieldIndex(tree_node *decl, const Type *Ty);

/// getINTEGER_CSTVal - Return the specified INTEGER_CST value as a uint64_t.
///
uint64_t getINTEGER_CSTVal(tree_node *exp);

/// isInt64 - Return true if t is an INTEGER_CST that fits in a 64 bit integer.
/// If Unsigned is false, returns whether it fits in a int64_t.  If Unsigned is
/// true, returns whether the value is non-negative and fits in a uint64_t.
/// Always returns false for overflowed constants or if t is NULL.
bool isInt64(tree_node *t, bool Unsigned);

/// getInt64 - Extract the value of an INTEGER_CST as a 64 bit integer.  If
/// Unsigned is false, the value must fit in a int64_t.  If Unsigned is true,
/// the value must be non-negative and fit in a uint64_t.  Must not be used on
/// overflowed constants.  These conditions can be checked by calling isInt64.
uint64_t getInt64(tree_node *t, bool Unsigned);

/// isPassedByInvisibleReference - Return true if the specified type should be
/// passed by 'invisible reference'. In other words, instead of passing the
/// thing by value, pass the address of a temporary.
bool isPassedByInvisibleReference(tree_node *type);

/// isSequentialCompatible - Return true if the specified gcc array, pointer or
/// vector type and the corresponding LLVM SequentialType lay out their elements
/// identically in memory, so doing a GEP accesses the right memory location.
/// We assume that objects without a known size do not.
extern bool isSequentialCompatible(tree_node *type);

/// OffsetIsLLVMCompatible - Return true if the given field is offset from the
/// start of the record by a constant amount which is not humongously big.
extern bool OffsetIsLLVMCompatible(tree_node *field_decl);

#define NO_LENGTH (~(uint64_t)0)

/// ArrayLengthOf - Returns the length of the given gcc array type, or NO_LENGTH
/// if the array has variable or unknown length.
extern uint64_t ArrayLengthOf(tree_node *type);

/// isBitfield - Returns whether to treat the specified field as a bitfield.
bool isBitfield(tree_node *field_decl);

/// getFieldOffsetInBits - Return the bit offset of a FIELD_DECL in a structure.
extern uint64_t getFieldOffsetInBits(tree_node *field);

/// ValidateRegisterVariable - Check that a static "asm" variable is
/// well-formed.  If not, emit error messages and return true.  If so, return
/// false.
bool ValidateRegisterVariable(tree_node *decl);

/// MemRef - This struct holds the information needed for a memory access:
/// a pointer to the memory, its alignment and whether the access is volatile.
class MemRef {
public:
  Value *Ptr;
  bool Volatile;
private:
  unsigned char LogAlign;
public:
  explicit MemRef() : Ptr(0), Volatile(false), LogAlign(0) {}
  explicit MemRef(Value *P, uint32_t A, bool V) : Ptr(P), Volatile(V) {
    // Forbid alignment 0 along with non-power-of-2 alignment values.
    assert(isPowerOf2_32(A) && "Alignment not a power of 2!");
    LogAlign = Log2_32(A);
  }

  uint32_t getAlignment() const {
    return 1U << LogAlign;
  }

  void setAlignment(uint32_t A) {
    LogAlign = Log2_32(A);
  }
};

/// LValue - This struct represents an lvalue in the program.  In particular,
/// the Ptr member indicates the memory that the lvalue lives in.  Alignment
/// is the alignment of the memory (in bytes).If this is a bitfield reference,
/// BitStart indicates the first bit in the memory that is part of the field
/// and BitSize indicates the extent.
///
/// "LValue" is intended to be a light-weight object passed around by-value.
class LValue : public MemRef {
public:
  unsigned char BitStart;
  unsigned char BitSize;
public:
  explicit LValue() : BitStart(255), BitSize(255) {}
  explicit LValue(MemRef &M) : MemRef(M), BitStart(255), BitSize(255) {}
  LValue(Value *P, uint32_t A, bool V = false) :
      MemRef(P, A, V), BitStart(255), BitSize(255) {}
  LValue(Value *P, uint32_t A, unsigned BSt, unsigned BSi, bool V = false) :
      MemRef(P, A, V), BitStart(BSt), BitSize(BSi) {
    assert(BitStart == BSt && BitSize == BSi &&
           "Bit values larger than 256?");
  }

  bool isBitfield() const { return BitStart != 255; }
};

/// PhiRecord - This struct holds the LLVM PHI node associated with a GCC phi.
struct PhiRecord {
  gimple_statement_d *gcc_phi;
  PHINode *PHI;
};

/// TreeToLLVM - An instance of this class is created and used to convert the
/// body of each function to LLVM.
///
class TreeToLLVM {
  // State that is initialized when the function starts.
  const TargetData &TD;
  tree_node *FnDecl;
  Function *Fn;
  BasicBlock *ReturnBB;
  unsigned ReturnOffset;

  // State that changes as the function is emitted.

  /// Builder - Instruction creator, the location to insert into is always the
  /// same as &Fn->back().
  LLVMBuilder Builder;

  // AllocaInsertionPoint - Place to insert alloca instructions.  Lazily created
  // and managed by CreateTemporary.
  Instruction *AllocaInsertionPoint;

  // SSAInsertionPoint - Place to insert reads corresponding to SSA default
  // definitions.
  Instruction *SSAInsertionPoint;

  /// BasicBlocks - Map from GCC to LLVM basic blocks.
  DenseMap<basic_block_def *, BasicBlock*> BasicBlocks;

  /// LocalDecls - Map from local declarations to their associated LLVM values.
  DenseMap<tree_node *, AssertingVH<Value> > LocalDecls;

  /// PendingPhis - Phi nodes which have not yet been populated with operands.
  SmallVector<PhiRecord, 16> PendingPhis;

  // SSANames - Map from GCC ssa names to the defining LLVM value.
  DenseMap<tree_node *, TrackingVH<Value> > SSANames;

public:

  //===---------------------- Local Declarations --------------------------===//

  /// DECL_LOCAL - Like DECL_LLVM, returns the LLVM declaration of a variable or
  /// function.  However DECL_LOCAL can be used with declarations local to the
  /// current function as well as with global declarations.
  Value *make_decl_local(tree_node *);
  #define DECL_LOCAL(NODE) make_decl_local(NODE)

  /// DEFINITION_LOCAL - Like DEFINITION_LLVM, ensures that the initial value or
  /// body of a variable or function will be output.  However DEFINITION_LOCAL
  /// can be used with declarations local to the current function as well as
  /// with global declarations.
  Value *make_definition_local(tree_node *);
  #define DEFINITION_LOCAL(NODE) make_definition_local(NODE)

  /// SET_DECL_LOCAL - Set the DECL_LOCAL for NODE to LLVM.
  Value *set_decl_local(tree_node *, Value *);
  #define SET_DECL_LOCAL(NODE, LLVM) set_decl_local(NODE, LLVM)

  /// DECL_LOCAL_IF_SET - The DECL_LOCAL for NODE, if it is set, or NULL, if it
  /// is not set.
  Value *get_decl_local(tree_node *);
  #define DECL_LOCAL_IF_SET(NODE) (HAS_RTL_P(NODE) ? get_decl_local(NODE) : NULL)

  /// DECL_LOCAL_SET_P - Returns nonzero if the DECL_LOCAL for NODE has already
  /// been set.
  #define DECL_LOCAL_SET_P(NODE) (DECL_LOCAL_IF_SET(NODE) != NULL)


private:

  //===---------------------- Exception Handling --------------------------===//

  /// NormalInvokes - Mapping from landing pad number to the set of invoke
  /// instructions that unwind to that landing pad.
  SmallVector<SmallVector<InvokeInst *, 8>, 16> NormalInvokes;

  /// ExceptionPtrs - Mapping from EH region index to the local holding the
  /// exception pointer for that region.
  SmallVector<AllocaInst *, 16> ExceptionPtrs;

  /// ExceptionFilters - Mapping from EH region index to the local holding the
  /// filter value for that region.
  SmallVector<AllocaInst *, 16> ExceptionFilters;

  /// FailureBlocks - Mapping from the index of a must-not-throw EH region to
  /// the block containing the failure code for the region (the code that is
  /// run if an exception is thrown in this region).
  SmallVector<BasicBlock *, 16> FailureBlocks;

  /// RewindBB - Block containing code that continues unwinding an exception.
  BasicBlock *RewindBB;

  /// RewindTmp - Local holding the exception to continue unwinding.
  AllocaInst *RewindTmp;

public:
  TreeToLLVM(tree_node *fndecl);
  ~TreeToLLVM();

  /// getFUNCTION_DECL - Return the FUNCTION_DECL node for the current function
  /// being compiled.
  tree_node *getFUNCTION_DECL() const { return FnDecl; }

  /// EmitFunction - Convert 'fndecl' to LLVM code.
  Function *EmitFunction();

  /// EmitBasicBlock - Convert the given basic block.
  void EmitBasicBlock(basic_block_def *bb);

  /// EmitLV - Convert the specified l-value tree node to LLVM code, returning
  /// the address of the result.
  LValue EmitLV(tree_node *exp);

  /// CastToAnyType - Cast the specified value to the specified type regardless
  /// of the types involved. This is an inferred cast.
  Value *CastToAnyType (Value *V, bool VSigned, const Type *Ty, bool TySigned);

  /// CastToUIntType - Cast the specified value to the specified type assuming
  /// that V's type and Ty are integral types. This arbitrates between BitCast,
  /// Trunc and ZExt.
  Value *CastToUIntType(Value *V, const Type *Ty);

  /// CastToSIntType - Cast the specified value to the specified type assuming
  /// that V's type and Ty are integral types. This arbitrates between BitCast,
  /// Trunc and SExt.
  Value *CastToSIntType(Value *V, const Type *Ty);

  /// CastToFPType - Cast the specified value to the specified type assuming
  /// that V's type and Ty are floating point types. This arbitrates between
  /// BitCast, FPTrunc and FPExt.
  Value *CastToFPType(Value *V, const Type *Ty);

  /// CreateAnyAdd - Add two LLVM scalar values with the given GCC type.  Does
  /// not support complex numbers.  The type is used to set overflow flags.
  Value *CreateAnyAdd(Value *LHS, Value *RHS, tree_node *type);

  /// CreateAnyMul - Multiply two LLVM scalar values with the given GCC type.
  /// Does not support complex numbers.  The type is used to set overflow flags.
  Value *CreateAnyMul(Value *LHS, Value *RHS, tree_node *type);

  /// CreateAnyNeg - Negate an LLVM scalar value with the given GCC type.  Does
  /// not support complex numbers.  The type is used to set overflow flags.
  Value *CreateAnyNeg(Value *V, tree_node *type);

  /// CreateAnySub - Subtract two LLVM scalar values with the given GCC type.
  /// Does not support complex numbers.
  Value *CreateAnySub(Value *LHS, Value *RHS, tree_node *type);

  /// CreateTemporary - Create a new alloca instruction of the specified type,
  /// inserting it into the entry block and returning it.  The resulting
  /// instruction's type is a pointer to the specified type.
  AllocaInst *CreateTemporary(const Type *Ty, unsigned align=0);

  /// CreateTempLoc - Like CreateTemporary, but returns a MemRef.
  MemRef CreateTempLoc(const Type *Ty);

  /// EmitAggregateCopy - Copy the elements from SrcLoc to DestLoc, using the
  /// GCC type specified by GCCType to know which elements to copy.
  void EmitAggregateCopy(MemRef DestLoc, MemRef SrcLoc, tree_node *GCCType);

  /// EmitAggregate - Store the specified tree node into the location given by
  /// DestLoc.
  void EmitAggregate(tree_node *exp, const MemRef &DestLoc);

private: // Helper functions.

  /// StartFunctionBody - Start the emission of 'fndecl', outputing all
  /// declarations for parameters and setting things up.
  void StartFunctionBody();

  /// FinishFunctionBody - Once the body of the function has been emitted, this
  /// cleans up and returns the result function.
  Function *FinishFunctionBody();

  /// PopulatePhiNodes - Populate generated phi nodes with their operands.
  void PopulatePhiNodes();

  /// getBasicBlock - Find or create the LLVM basic block corresponding to BB.
  BasicBlock *getBasicBlock(basic_block_def *bb);

  /// getLabelDeclBlock - Lazily get and create a basic block for the specified
  /// label.
  BasicBlock *getLabelDeclBlock(tree_node *LabelDecl);

  /// DefineSSAName - Use the given value as the definition of the given SSA
  /// name.  Returns the provided value as a convenience.
  Value *DefineSSAName(tree_node *reg, Value *Val);

  /// BeginBlock - Add the specified basic block to the end of the function.  If
  /// the previous block falls through into it, add an explicit branch.
  void BeginBlock(BasicBlock *BB);

  /// EmitAggregateZero - Zero the elements of DestLoc.
  void EmitAggregateZero(MemRef DestLoc, tree_node *GCCType);

  /// EmitMemCpy/EmitMemMove/EmitMemSet - Emit an llvm.memcpy/llvm.memmove or
  /// llvm.memset call with the specified operands.  Returns DestPtr bitcast
  /// to i8*.
  Value *EmitMemCpy(Value *DestPtr, Value *SrcPtr, Value *Size, unsigned Align);
  Value *EmitMemMove(Value *DestPtr, Value *SrcPtr, Value *Size, unsigned Align);
  Value *EmitMemSet(Value *DestPtr, Value *SrcVal, Value *Size, unsigned Align);

  /// EmitLandingPads - Emit EH landing pads.
  void EmitLandingPads();

  /// EmitFailureBlocks - Emit the blocks containing failure code executed when
  /// an exception is thrown in a must-not-throw region.
  void EmitFailureBlocks();

  /// EmitRewindBlock - Emit the block containing code to continue unwinding an
  /// exception.
  void EmitRewindBlock();

  /// EmitDebugInfo - Return true if debug info is to be emitted for current
  /// function.
  bool EmitDebugInfo();

private: // Helpers for exception handling.

  /// getLandingPad - Return the landing pad for the given exception handling
  /// region, creating it if necessary.
  BasicBlock *getLandingPad(unsigned RegionNo);

  /// getExceptionPtr - Return the local holding the exception pointer for the
  /// given exception handling region, creating it if necessary.
  AllocaInst *getExceptionPtr(unsigned RegionNo);

  /// getExceptionFilter - Return the local holding the filter value for the
  /// given exception handling region, creating it if necessary.
  AllocaInst *getExceptionFilter(unsigned RegionNo);

  /// getFailureBlock - Return the basic block containing the failure code for
  /// the given exception handling region, creating it if necessary.
  BasicBlock *getFailureBlock(unsigned RegionNo);

private:
  void EmitAutomaticVariableDecl(tree_node *decl);

  /// EmitAnnotateIntrinsic - Emits call to annotate attr intrinsic
  void EmitAnnotateIntrinsic(Value *V, tree_node *decl);

  /// EmitTypeGcroot - Emits call to make type a gcroot
  void EmitTypeGcroot(Value *V);

private:

  //===------------------ Render* - Convert GIMPLE to LLVM ----------------===//

  void RenderGIMPLE_ASM(gimple_statement_d *stmt);
  void RenderGIMPLE_ASSIGN(gimple_statement_d *stmt);
  void RenderGIMPLE_CALL(gimple_statement_d *stmt);
  void RenderGIMPLE_COND(gimple_statement_d *stmt);
  void RenderGIMPLE_EH_DISPATCH(gimple_statement_d *stmt);
  void RenderGIMPLE_GOTO(gimple_statement_d *stmt);
  void RenderGIMPLE_RESX(gimple_statement_d *stmt);
  void RenderGIMPLE_RETURN(gimple_statement_d *stmt);
  void RenderGIMPLE_SWITCH(gimple_statement_d *stmt);

  // Render helpers.

  /// EmitAssignRHS - Convert the RHS of a scalar GIMPLE_ASSIGN to LLVM.
  Value *EmitAssignRHS(gimple_statement_d *stmt);

  /// EmitAssignSingleRHS - Helper for EmitAssignRHS.  Handles those RHS that
  /// are not register expressions.
  Value *EmitAssignSingleRHS(tree_node *rhs);

  /// OutputCallRHS - Convert the RHS of a GIMPLE_CALL.
  Value *OutputCallRHS(gimple_statement_d *stmt, const MemRef *DestLoc);

  /// WriteScalarToLHS - Store RHS, a non-aggregate value, into the given LHS.
  void WriteScalarToLHS(tree_node *lhs, Value *Scalar);

private:

  //===---------- EmitReg* - Convert register expression to LLVM ----------===//

  /// UselesslyTypeConvert - The useless_type_conversion_p predicate implicitly
  /// defines the GCC middle-end type system.  For scalar GCC types inner_type
  /// and outer_type, if 'useless_type_conversion_p(outer_type, inner_type)' is
  /// true then the corresponding LLVM inner and outer types (see getRegType)
  /// are equal except possibly if they are both pointer types (casts to 'void*'
  /// are considered useless for example) or types derived from pointer types
  /// (vector types with pointer element type are the only possibility here).
  /// This method converts LLVM values of the inner type to the outer type.
  Value *UselesslyTypeConvert(Value *V, const Type *Ty) {
    return Builder.CreateBitCast(V, Ty);
  }

  /// EmitRegister - Convert the specified gimple register or local constant of
  /// register type to an LLVM value.  Only creates code in the entry block.
  Value *EmitRegister(tree_node *reg);

  /// EmitReg_SSA_NAME - Return the defining value of the given SSA_NAME.
  /// Only creates code in the entry block.
  Value *EmitReg_SSA_NAME(tree_node *reg);

  // Unary expressions.
  Value *EmitReg_ABS_EXPR(tree_node *op);
  Value *EmitReg_BIT_NOT_EXPR(tree_node *op);
  Value *EmitReg_CONJ_EXPR(tree_node *op);
  Value *EmitReg_CONVERT_EXPR(tree_node *type, tree_node *op);
  Value *EmitReg_NEGATE_EXPR(tree_node *op);
  Value *EmitReg_PAREN_EXPR(tree_node *exp);
  Value *EmitReg_TRUTH_NOT_EXPR(tree_node *type, tree_node *op);

  // Comparisons.

  /// EmitCompare - Compare LHS with RHS using the appropriate comparison code.
  /// The result is an i1 boolean.
  Value *EmitCompare(tree_node *lhs, tree_node *rhs, unsigned code);

  // Binary expressions.
  Value *EmitReg_MinMaxExpr(tree_node *type, tree_node *op0, tree_node *op1,
                            unsigned UIPred, unsigned SIPred, unsigned Opc,
                            bool isMax);
  Value *EmitReg_RotateOp(tree_node *type, tree_node *op0, tree_node *op1,
                          unsigned Opc1, unsigned Opc2);
  Value *EmitReg_ShiftOp(tree_node *op0, tree_node *op1, unsigned Opc);
  Value *EmitReg_TruthOp(tree_node *type, tree_node *op0, tree_node *op1,
                         unsigned Opc);
  Value *EmitReg_BIT_AND_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_BIT_IOR_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_BIT_XOR_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_CEIL_DIV_EXPR(tree_node *type, tree_node *op0, tree_node *op1);
  Value *EmitReg_COMPLEX_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_FLOOR_DIV_EXPR(tree_node *type, tree_node *op0,
                                tree_node *op1);
  Value *EmitReg_FLOOR_MOD_EXPR(tree_node *type, tree_node *op0,
                                tree_node *op1);
  Value *EmitReg_MINUS_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_MULT_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_PLUS_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_POINTER_PLUS_EXPR(tree_node *type, tree_node *op0,
                                   tree_node *op1);
  Value *EmitReg_RDIV_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_ROUND_DIV_EXPR(tree_node *type, tree_node *op0,
                                tree_node *op1);
  Value *EmitReg_TRUNC_DIV_EXPR(tree_node *op0, tree_node *op1, bool isExact);
  Value *EmitReg_TRUNC_MOD_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_VEC_EXTRACT_EVEN_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_VEC_EXTRACT_ODD_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_VEC_INTERLEAVE_HIGH_EXPR(tree_node *op0, tree_node *op1);
  Value *EmitReg_VEC_INTERLEAVE_LOW_EXPR(tree_node *op0, tree_node *op1);

  Value *EmitLoadOfLValue(tree_node *exp);
  Value *EmitOBJ_TYPE_REF(tree_node *exp);
  Value *EmitADDR_EXPR(tree_node *exp);
  Value *EmitCOND_EXPR(tree_node *exp);
  Value *EmitCallOf(Value *Callee, gimple_statement_d *stmt,
                    const MemRef *DestLoc, const AttrListPtr &PAL);
  CallInst *EmitSimpleCall(StringRef CalleeName, tree_node *ret_type,
                           /* arguments */ ...) END_WITH_NULL;
  Value *EmitFieldAnnotation(Value *FieldPtr, tree_node *FieldDecl);

  // Inline Assembly and Register Variables.
  Value *EmitReadOfRegisterVariable(tree_node *vardecl);
  void EmitModifyOfRegisterVariable(tree_node *vardecl, Value *RHS);

  // Helpers for Builtin Function Expansion.
  void EmitMemoryBarrier(bool ll, bool ls, bool sl, bool ss, bool device);
  Value *BuildVector(const std::vector<Value*> &Elts);
  Value *BuildVector(Value *Elt, ...);
  Value *BuildVectorShuffle(Value *InVec1, Value *InVec2, ...);
  Value *BuildBinaryAtomicBuiltin(gimple_statement_d *stmt, Intrinsic::ID id);
  Value *BuildCmpAndSwapAtomicBuiltin(gimple_statement_d *stmt, tree_node *type,
                                      bool isBool);

  // Builtin Function Expansion.
  bool EmitBuiltinCall(gimple_statement_d *stmt, tree_node *fndecl,
                       const MemRef *DestLoc, Value *&Result);
  bool EmitFrontendExpandedBuiltinCall(gimple_statement_d *stmt,
                                       tree_node *fndecl, const MemRef *DestLoc,
                                       Value *&Result);
  bool EmitBuiltinUnaryOp(Value *InVal, Value *&Result, Intrinsic::ID Id);
  Value *EmitBuiltinSQRT(gimple_statement_d *stmt);
  Value *EmitBuiltinPOWI(gimple_statement_d *stmt);
  Value *EmitBuiltinPOW(gimple_statement_d *stmt);
  Value *EmitBuiltinLCEIL(gimple_statement_d *stmt);
  Value *EmitBuiltinLFLOOR(gimple_statement_d *stmt);

  bool EmitBuiltinConstantP(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinAlloca(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinExpect(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinExtendPointer(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinVAStart(gimple_statement_d *stmt);
  bool EmitBuiltinVAEnd(gimple_statement_d *stmt);
  bool EmitBuiltinVACopy(gimple_statement_d *stmt);
  bool EmitBuiltinMemCopy(gimple_statement_d *stmt, Value *&Result,
                          bool isMemMove, bool SizeCheck);
  bool EmitBuiltinMemSet(gimple_statement_d *stmt, Value *&Result,
                         bool SizeCheck);
  bool EmitBuiltinBZero(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinPrefetch(gimple_statement_d *stmt);
  bool EmitBuiltinReturnAddr(gimple_statement_d *stmt, Value *&Result,
                             bool isFrame);
  bool EmitBuiltinExtractReturnAddr(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinFrobReturnAddr(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinStackSave(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinStackRestore(gimple_statement_d *stmt);
  bool EmitBuiltinEHPointer(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinDwarfCFA(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinDwarfSPColumn(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinEHReturnDataRegno(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinEHReturn(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinInitDwarfRegSizes(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinUnwindInit(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinAdjustTrampoline(gimple_statement_d *stmt, Value *&Result);
  bool EmitBuiltinInitTrampoline(gimple_statement_d *stmt, Value *&Result);

  // Complex Math Expressions.
  Value *CreateComplex(Value *Real, Value *Imag);
  void SplitComplex(Value *Complex, Value *&Real, Value *&Imag);

  // L-Value Expressions.
  LValue EmitLV_ARRAY_REF(tree_node *exp);
  LValue EmitLV_BIT_FIELD_REF(tree_node *exp);
  LValue EmitLV_COMPONENT_REF(tree_node *exp);
  LValue EmitLV_DECL(tree_node *exp);
  LValue EmitLV_INDIRECT_REF(tree_node *exp);
  LValue EmitLV_MISALIGNED_INDIRECT_REF(tree_node *exp);
  LValue EmitLV_VIEW_CONVERT_EXPR(tree_node *exp);
  LValue EmitLV_WITH_SIZE_EXPR(tree_node *exp);
  LValue EmitLV_XXXXPART_EXPR(tree_node *exp, unsigned Idx);
  LValue EmitLV_SSA_NAME(tree_node *exp);
  LValue EmitLV_TARGET_MEM_REF(tree_node *exp);

  // Constant Expressions.
  Value *EmitINTEGER_CST(tree_node *exp);
  Value *EmitREAL_CST(tree_node *exp);
  Value *EmitCONSTRUCTOR(tree_node *exp, const MemRef *DestLoc);


  // Emit helpers.

  /// EmitMinInvariant - The given value is constant in this function.  Return
  /// the corresponding LLVM value. Only creates code in the entry block.
  Value *EmitMinInvariant(tree_node *reg);

  /// EmitInvariantAddress - The given address is constant in this function.
  /// Return the corresponding LLVM value. Only creates code in the entry block.
  Value *EmitInvariantAddress(tree_node *addr);

  /// EmitRegisterConstant - Convert the given global constant of register type
  /// to an LLVM constant.  Creates no code, only constants.
  Constant *EmitRegisterConstant(tree_node *reg);

  /// EmitComplexRegisterConstant - Turn the given COMPLEX_CST into an LLVM
  /// constant of the corresponding register type.
  Constant *EmitComplexRegisterConstant(tree_node *reg);

  /// EmitIntegerRegisterConstant - Turn the given INTEGER_CST into an LLVM
  /// constant of the corresponding register type.
  Constant *EmitIntegerRegisterConstant(tree_node *reg);

  /// EmitRealRegisterConstant - Turn the given REAL_CST into an LLVM constant
  /// of the corresponding register type.
  Constant *EmitRealRegisterConstant(tree_node *reg);

  /// EmitConstantVectorConstructor - Turn the given constant CONSTRUCTOR into
  /// an LLVM constant of the corresponding vector register type.
  Constant *EmitConstantVectorConstructor(tree_node *reg);

  /// EmitVectorRegisterConstant - Turn the given VECTOR_CST into an LLVM
  /// constant of the corresponding register type.
  Constant *EmitVectorRegisterConstant(tree_node *reg);

  /// Mem2Reg - Convert a value of in-memory type (that given by ConvertType)
  /// to in-register type (that given by getRegType).  TODO: Eliminate these
  /// methods: "memory" values should never be held in registers.  Currently
  /// this is mainly used for marshalling function parameters and return values,
  /// but that should be completely independent of the reg vs mem value logic.
  Value *Mem2Reg(Value *V, tree_node *type, LLVMBuilder &Builder);

  /// Reg2Mem - Convert a value of in-register type (that given by getRegType)
  /// to in-memory type (that given by ConvertType).  TODO: Eliminate this
  /// method: "memory" values should never be held in registers.  Currently
  /// this is mainly used for marshalling function parameters and return values,
  /// but that should be completely independent of the reg vs mem value logic.
  Value *Reg2Mem(Value *V, tree_node *type, LLVMBuilder &Builder);

  /// EmitMemory - Convert the specified gimple register or local constant of
  /// register type to an LLVM value with in-memory type (given by ConvertType).
  /// TODO: Eliminate this method, see Mem2Reg and Reg2Mem above.
  Value *EmitMemory(tree_node *reg);

  /// LoadRegisterFromMemory - Loads a value of the given scalar GCC type from
  /// the memory location pointed to by Loc.  Takes care of adjusting for any
  /// differences between in-memory and in-register types (the returned value
  /// is of in-register type, as returned by getRegType).
  Value *LoadRegisterFromMemory(MemRef Loc, tree_node *type,
                                LLVMBuilder &Builder);

  /// StoreRegisterToMemory - Stores the given value to the memory pointed to by
  /// Loc.  Takes care of adjusting for any differences between the value's type
  /// (which is the in-register type given by getRegType) and the in-memory type.
  void StoreRegisterToMemory(Value *V, MemRef Loc, tree_node *type,
                             LLVMBuilder &Builder);

private:
  // Optional target defined builtin intrinsic expanding function.
  bool TargetIntrinsicLower(gimple_statement_d *stmt,
                            tree_node *fndecl,
                            const MemRef *DestLoc,
                            Value *&Result,
                            const Type *ResultType,
                            std::vector<Value*> &Ops);

public:
  // Helper for taking the address of a label.
  Constant *AddressOfLABEL_DECL(tree_node *exp);
};

#endif /* DRAGONEGG_INTERNALS_H */
