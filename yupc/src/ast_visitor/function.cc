#include <visitor.h>
#include <util.h>


class FuncParam
{
public:
    llvm::Type* paramType;
    std::string paramName;

    FuncParam(llvm::Type* type, std::string pn)
    {
        paramType = type;
        paramName = pn;
    }
};

std::any Visitor::visitFunc_return(YupParser::Func_returnContext *ctx)
{
    std::any value = this->visit(ctx->expr());
    return value;
}

std::any Visitor::visitFunc_def(YupParser::Func_defContext *ctx)
{
    std::string funcName = ctx
            ->func_signature()
            ->IDENTIFIER()
            ->getText();

    llvm::Function* function = std::any_cast<llvm::Function*>(this->visit(ctx->func_signature()));

    if (!function)
    {
        fprintf(stderr, "cannot get signature for function \"%s\"\n", funcName.c_str());
        exit(1);
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(codegenCtx, "entry", function);
    irBuilder.SetInsertPoint(block);

    symbolTable.clear();

    for (auto &arg : function->args())
    {
        symbolTable[arg.getName().str()] = &arg;
    }

    if (!function->getFunctionType()->getReturnType()->isVoidTy())
    {
        int blockStatementCount = ctx->code_block()->statement().size();
        std::any retResult = this->visit(ctx->code_block()->statement()[blockStatementCount - 1]);
        llvm::Value* retValue = std::any_cast<llvm::Value*>(retResult);

        // TODO: fix return statement segfault
        //irBuilder.CreateRet(retValue);
    }
    else
    {
        irBuilder.CreateRetVoid();
    }

    llvm::verifyFunction(*function, &llvm::outs());

    return function;
}

std::any Visitor::visitFunc_signature(YupParser::Func_signatureContext *ctx)
{
    std::string name = ctx->IDENTIFIER()->getText();
    std::any typeAnnot = this->visit(ctx->type_annot());
    std::string retTypeName = std::any_cast<std::string>(typeAnnot);
    llvm::Type* returnType = matchType(retTypeName);

    std::vector<FuncParam*> params;
    for (const auto p : ctx->func_param())
    {
        FuncParam* fp = std::any_cast<FuncParam*>(this->visit(p));
        params.push_back(fp);
    }

    std::vector<llvm::Type*> paramTypes;
    for (const auto pt : params)
    {
        paramTypes.push_back(pt->paramType);
    }

    llvm::FunctionType *functionType = llvm::FunctionType::get(
            returnType,
            paramTypes,
            false);

    llvm::Function* function = llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            name,
            module.get());

    unsigned idx = 0;
    for (auto &arg : function->args())
    {
        arg.setName(params[idx++]->paramName);
    }

    return function;
}

std::any Visitor::visitFunc_param(YupParser::Func_paramContext *ctx) {
    std::any typeAnnot = this->visit(ctx->type_annot());
    std::string typeName = std::any_cast<std::string>(typeAnnot);
    llvm::Type* type = matchType(typeName);

    FuncParam* funcParam = new FuncParam(type, typeName);
    return funcParam;
}