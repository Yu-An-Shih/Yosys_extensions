#ifndef WRITE_LLVM_H
#define WRITE_LLVM_H
  
// LLVM headers needed below
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

// Without this, yosys.h gets confused by the above LLVM headers.  Strange!!!
// It seems to be caused by the identifier "ID" being used in clever ways by both packages.
#include "llvm/IR/PassManager.h"

// Yosys headers
#include "kernel/yosys.h"

#include "driver_tools.h"

class LLVMWriter {

public:

  struct Options {
    bool verbose_llvm_value_names = false;
    bool cell_based_llvm_value_names = true;
    bool simplify_and_or_gates = true;
    bool simplify_muxes = true;
    bool use_poison = false;
  };

  LLVMWriter(const Options& options);
  ~LLVMWriter();

  void write_llvm_ir(Yosys::RTLIL::Module *unrolledRtlMod,
                      std::string targetName,  // As specified in allowed_target.txt
                      bool targetIsVec,       // target is ASV vector
                      std::string modName,  // from original Verilog, e.g. "M8080"
                      int num_cycles,
                      std::string llvmFileName,
                      std::string funcName);

  void reset();

private:

  class ValueCache {
    public:
      void add(llvm::Value *value, const DriverSpec& driver);
      llvm::Value *find(const DriverSpec& driver);
      void clear() { _dict.clear(); _nHits = 0; _nMisses = 0; }
      size_t size() { return _dict.size(); }
      size_t nHits() const { return _nHits; }
      size_t nMisses() const { return _nMisses; }

    private:
      Yosys::dict<DriverSpec, llvm::Value*> _dict;

      size_t _nHits = 0;
      size_t _nMisses = 0;
  };


  std::shared_ptr<llvm::IRBuilder<>> b;
  std::shared_ptr<llvm::LLVMContext> c;
  std::shared_ptr<llvm::Module> llvmMod;
  llvm::Function *llvmFunc;  // function being generated

  ValueCache valueCache;
  DriverFinder finder;
  Options opts;


  llvm::IntegerType *llvmWidth(unsigned a);


  // Dangerous: only supports up to 64 bits.
  llvm::ConstantInt *llvmInt(uint64_t val, unsigned width);


  // More useful
  llvm::ConstantInt *llvmZero(unsigned width);

  llvm::PoisonValue *llvmPoison(unsigned width);
  llvm::UndefValue *llvmUndef(unsigned width);

  unsigned getWidth(llvm::Value *val);

  // Generate a value for a primary input 
  llvm::Value *generatePrimaryInputValue(Yosys::RTLIL::Wire *port);

  // Find or create a Value representing what drives the given input port of the given cell.
  llvm::Value *generateInputValue(Yosys::RTLIL::Cell *cell,
                                  Yosys::RTLIL::IdString port);

  llvm::Value* generateLoad(llvm::Value *array, unsigned elementWidth, unsigned idx,
                             Yosys::RTLIL::IdString valueName);

  void generateStore(llvm::Value *array, unsigned idx, llvm::Value *val);

  // Helpers for generateCellOutputValue() below
  llvm::Value *generateSimplifiedAndCellOutputValue(llvm::Value *valA, llvm::Value *valB);
  llvm::Value *generateAndCellOutputValue(llvm::Value *valA, llvm::Value *valB);
  llvm::Value *generateUnaryCellOutputValue(Yosys::RTLIL::Cell *cell);
  llvm::Value *generateBinaryCellOutputValue(Yosys::RTLIL::Cell *cell);
  llvm::Value *generateSimplifiedMuxCellOutputValue(Yosys::RTLIL::Cell *cell);
  llvm::Value *generateMuxCellOutputValue(Yosys::RTLIL::Cell *cell);
  llvm::Value *generatePmuxCellOutputValue(Yosys::RTLIL::Cell *cell);


  // Create a Value representing the output port of the given cell.
  // Since this is not given a DriverSpec, it does not touch the valueCache.
  // The caller is reponsible for that.
  // TODO: Should it instead make a temporary DriverSpec to access the valueCache?
  llvm::Value *generateCellOutputValue(Yosys::RTLIL::Cell *cell,
                                       Yosys::RTLIL::IdString port);


  // Generate the value of the given chunk, which is constant, or a
  // slice of a single wire or cell output.  The result will be offset
  // by the given amount, and zero-extended to totalWidth.
  llvm::Value *generateChunkValue(const DriverChunk& chunk,
                             int totalWidth, int offset);


  llvm::Value *generateValue(const DriverSpec& dSpec);


  // The wire represents a target ASV, and is not NOT necessarily a port
  llvm::Value *generateDestValue(Yosys::RTLIL::Wire *wire);

  llvm::Function*
  generateFunctionDecl(const std::string& funcName, Yosys::RTLIL::Module *mod,
                       const Yosys::dict<std::string, unsigned>& targetVectors,
                       int retWidth);


};


#endif
