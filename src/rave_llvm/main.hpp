#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

struct RaveContext {
	LLVMContextRef	 context;
	LLVMModuleRef	 module;
	LLVMBuilderRef	 builder;
	LLVMDIBuilderRef dibuilder;
	LLVMValueRef	 currentFunction;
	LLVMTypeRef		 currentFunctionType;

	LLVMMetadataRef compileUnit;
	LLVMMetadataRef fileMeta;
	LLVMMetadataRef intType;
	LLVMMetadataRef subroutineType;

	LLVMTargetMachineRef targetMachine;
	std::string			 objectPath;
	std::string			 outputPath;
};

#define DW_ATE_signed 0x05

static void
die(const char* msg) {
	fprintf(stderr, "FATAL: %s\n", msg);
	exit(1);
}

static void
rave_init(
	RaveContext* rc,
	const char*	 moduleName,
	const char*	 sourcePath
) {
	rc->context				= LLVMContextCreate();
	rc->module				= LLVMModuleCreateWithNameInContext(moduleName, rc->context);
	rc->builder				= LLVMCreateBuilderInContext(rc->context);
	rc->currentFunction		= nullptr;
	rc->currentFunctionType = nullptr;
	rc->objectPath			= std::string(moduleName) + ".o";
	rc->outputPath			= std::string(moduleName) + ".exe";

	LLVMSetIsNewDbgInfoFormat(rc->module, true);

	LLVMSetSourceFileName(rc->module, sourcePath, strlen(sourcePath));

	rc->dibuilder = LLVMCreateDIBuilder(rc->module);

	rc->fileMeta = LLVMDIBuilderCreateFile(rc->dibuilder, sourcePath, strlen(sourcePath), ".", 1);

	rc->compileUnit = LLVMDIBuilderCreateCompileUnit(
		rc->dibuilder, LLVMDWARFSourceLanguageC, rc->fileMeta, "rave", 4, 0, "", 0, 0, "", 0,
		LLVMDWARFEmissionFull, 0, 1, 0, "", 0, "", 0
	);

	rc->intType =
		LLVMDIBuilderCreateBasicType(rc->dibuilder, "int", 3, 32, DW_ATE_signed, LLVMDIFlagZero);

	LLVMMetadataRef subTypes[] = {rc->intType, rc->intType, rc->intType};
	rc->subroutineType =
		LLVMDIBuilderCreateSubroutineType(rc->dibuilder, rc->fileMeta, subTypes, 3, LLVMDIFlagZero);
}

static void
rave_create_param_record(
	RaveContext* rc,
	const char*	 name,
	LLVMValueRef alloca,
	unsigned	 argNo,
	unsigned	 line,
	unsigned	 column
) {
	LLVMMetadataRef paramMeta = LLVMDIBuilderCreateParameterVariable(
		rc->dibuilder, LLVMGetSubprogram(rc->currentFunction), name, strlen(name), argNo,
		rc->fileMeta, line, rc->intType, 1, LLVMDIFlagZero
	);

	LLVMMetadataRef debugLoc = LLVMDIBuilderCreateDebugLocation(
		rc->context, line, column, LLVMGetSubprogram(rc->currentFunction), nullptr
	);

	LLVMDIBuilderInsertDeclareRecordAtEnd(
		rc->dibuilder, alloca, paramMeta, LLVMDIBuilderCreateExpression(rc->dibuilder, nullptr, 0),
		debugLoc, LLVMGetInsertBlock(rc->builder)
	);
}

static void
rave_start_function(
	RaveContext* rc,
	const char*	 name,
	unsigned	 line
) {
	LLVMTypeRef paramTypes[] = {
		LLVMInt32TypeInContext(rc->context), LLVMInt32TypeInContext(rc->context)
	};
	rc->currentFunctionType =
		LLVMFunctionType(LLVMInt32TypeInContext(rc->context), paramTypes, 2, 0);

	LLVMValueRef func = LLVMAddFunction(rc->module, name, rc->currentFunctionType);

	LLVMMetadataRef sub = LLVMDIBuilderCreateFunction(
		rc->dibuilder, rc->fileMeta, name, strlen(name), name, strlen(name), rc->fileMeta, line,
		rc->subroutineType, 0, 1, line, LLVMDIFlagZero, 0
	);

	LLVMSetSubprogram(func, sub);

	LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(rc->context, func, "entry");
	LLVMPositionBuilderAtEnd(rc->builder, entry);

	rc->currentFunction = func;

	LLVMValueRef argA = LLVMGetParam(func, 0);
	LLVMValueRef argB = LLVMGetParam(func, 1);
	LLVMSetValueName2(argA, "a", 1);
	LLVMSetValueName2(argB, "b", 1);

	LLVMValueRef allocaA =
		LLVMBuildAlloca(rc->builder, LLVMInt32TypeInContext(rc->context), "a.addr");
	LLVMValueRef allocaB =
		LLVMBuildAlloca(rc->builder, LLVMInt32TypeInContext(rc->context), "b.addr");

	LLVMBuildStore(rc->builder, argA, allocaA);
	LLVMBuildStore(rc->builder, argB, allocaB);

	rave_create_param_record(rc, "a", allocaA, 1, line, 0);
	rave_create_param_record(rc, "b", allocaB, 2, line, 8);

	LLVMValueRef loadA =
		LLVMBuildLoad2(rc->builder, LLVMInt32TypeInContext(rc->context), allocaA, "load_a");
	LLVMValueRef loadB =
		LLVMBuildLoad2(rc->builder, LLVMInt32TypeInContext(rc->context), allocaB, "load_b");
	LLVMValueRef sum = LLVMBuildAdd(rc->builder, loadA, loadB, "sum");

	LLVMBuildRet(rc->builder, sum);
}

static void
rave_create_main_wrapper(RaveContext* rc) {
	LLVMTypeRef mainParamTypes[] = {
		LLVMInt32TypeInContext(rc->context),
		LLVMPointerType(LLVMPointerType(LLVMInt8TypeInContext(rc->context), 0), 0)
	};
	LLVMTypeRef mainType =
		LLVMFunctionType(LLVMInt32TypeInContext(rc->context), mainParamTypes, 2, 0);

	LLVMValueRef mainFunc = LLVMAddFunction(rc->module, "main", mainType);

	LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(rc->context, mainFunc, "entry");
	LLVMPositionBuilderAtEnd(rc->builder, entry);

	LLVMValueRef ten	= LLVMConstInt(LLVMInt32TypeInContext(rc->context), 10, 0);
	LLVMValueRef twenty = LLVMConstInt(LLVMInt32TypeInContext(rc->context), 20, 0);

	LLVMValueRef addFunc = LLVMGetNamedFunction(rc->module, "add");
	if (!addFunc)
		die("add function not found");

	LLVMValueRef callArgs[] = {ten, twenty};
	LLVMValueRef result =
		LLVMBuildCall2(rc->builder, rc->currentFunctionType, addFunc, callArgs, 2, "result");

	LLVMBuildRet(rc->builder, result);
}

static void
rave_finalize_debug(RaveContext* rc) {
	LLVMDIBuilderFinalize(rc->dibuilder);
}

static void
rave_verify(RaveContext* rc) {
	char* error = nullptr;
	if (LLVMVerifyModule(rc->module, LLVMReturnStatusAction, &error)) {
		fprintf(stderr, "VERIFY ERROR: %s\n", error);
		LLVMDisposeMessage(error);
		exit(1);
	}
	LLVMDisposeMessage(error);
}

static void
rave_print_module(RaveContext* rc) {
	char* ir = LLVMPrintModuleToString(rc->module);
	puts(ir);
	LLVMDisposeMessage(ir);
}

static void
rave_init_target(RaveContext* rc) {
	if (LLVMInitializeNativeTarget())
		die("no native target available");
	if (LLVMInitializeNativeAsmPrinter())
		die("no native asm printer available");
	if (LLVMInitializeNativeAsmParser())
		die("no native asm parser available");

	const char* triple = LLVMGetDefaultTargetTriple();
	LLVMSetTarget(rc->module, triple);

	LLVMTargetRef target = nullptr;
	char*		  err	 = nullptr;
	if (LLVMGetTargetFromTriple(triple, &target, &err)) {
		fprintf(stderr, "TARGET ERROR: %s\n", err);
		LLVMDisposeMessage(err);
		exit(1);
	}
	LLVMDisposeMessage(err);

	rc->targetMachine = LLVMCreateTargetMachine(
		target, triple, "generic", "", LLVMCodeGenLevelDefault, LLVMRelocDefault,
		LLVMCodeModelDefault
	);
}

static void
rave_emit_object(RaveContext* rc) {
	char* err = nullptr;
	if (LLVMTargetMachineEmitToFile(
			rc->targetMachine, rc->module, rc->objectPath.data(), LLVMObjectFile, &err
		)) {
		fprintf(stderr, "EMIT ERROR: %s\n", err);
		LLVMDisposeMessage(err);
		exit(1);
	}
	LLVMDisposeMessage(err);
}

static void
rave_link_executable(RaveContext* rc) {
	std::string cmd = "clang " + rc->objectPath + " -o " + rc->outputPath + " -g";
	int			ret = system(cmd.c_str());
	if (ret != 0) {
		fprintf(stderr, "LINK FAILED with code %d\n", ret);
		exit(1);
	}
}

static void
rave_run_executable(RaveContext* rc) {
	std::string cmd = rc->outputPath;
	int			ret = system(cmd.c_str());
	printf("\nprogram exited with code %d\n", ret);
}

static void
rave_cleanup(RaveContext* rc) {
	LLVMDisposeBuilder(rc->builder);
	LLVMDisposeModule(rc->module);
	LLVMContextDispose(rc->context);
	if (rc->targetMachine) {
		LLVMDisposeTargetMachine(rc->targetMachine);
	}
}

int
main() {
	RaveContext rc{};

	rave_init(&rc, "rave_demo", "rave_demo.c");

	rave_start_function(&rc, "add", 1);

	rave_create_main_wrapper(&rc);

	rave_finalize_debug(&rc);

	rave_verify(&rc);

	rave_print_module(&rc);

	rave_init_target(&rc);

	rave_emit_object(&rc);

	rave_link_executable(&rc);

	rave_run_executable(&rc);

	rave_cleanup(&rc);

	return 0;
}
