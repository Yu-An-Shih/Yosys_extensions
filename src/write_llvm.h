#ifndef WRITE_LLVM_H
#define WRITE_LLVM_H
  
// LLVM headers needed below
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"

// Without this, yosys.h gets confused by the above LLVM headers.  Strange!!!
// It seems to be caused by the word "ID" being used in clever ways by both packages.
#include "llvm/IR/PassManager.h"





// Yosys headers
#include "kernel/yosys.h"
//#include "kernel/rtlil.h"


llvm::Value *generateValue(Yosys::RTLIL::Wire *wire,
                           std::shared_ptr<llvm::LLVMContext> c,
                           std::shared_ptr<llvm::IRBuilder<>> b);


void write_llvm_ir(Yosys::RTLIL::Module *unrolledRtlMod,
                   std::string modName, std::string destName, std::string llvmFileName);

#endif