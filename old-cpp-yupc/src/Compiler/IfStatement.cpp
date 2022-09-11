#include "Compiler/Visitor.h"
#include "Compiler/CompilationUnit.h"
#include "Parser/YupParser.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

#include <any>
#include <iostream>

std::any yupc::Visitor::visitIfStatement(yupc::YupParser::IfStatementContext *ctx)
{
	this->visit(ctx->expression());	
	llvm::Value *condition 	   = yupc::CompilationUnits.back()->ValueStack.top();
	llvm::Value *conditionBool = yupc::CompilationUnits.back()->IRBuilder->CreateFCmpONE(condition,
		llvm::ConstantFP::get(*yupc::CompilationUnits.back()->Context, llvm::APFloat(0.0)), "");
		
	/*llvm::Function   *func    = yupc::CompilationUnits.back()->IRBuilder->GetInsertBlock()->getParent();
	llvm::BasicBlock *thenBB  = llvm::BasicBlock::Create(*yupc::CompilationUnits.back()->Context, "", func);
	llvm::BasicBlock *elseBB  = llvm::BasicBlock::Create(*yupc::CompilationUnits.back()->Context, "");
	llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*yupc::CompilationUnits.back()->Context, "");

	yupc::CompilationUnits.back()->IRBuilder->CreateCondBr(conditionBool, thenBB, elseBB);
	yupc::CompilationUnits.back()->IRBuilder->SetInsertPoint(thenBB);

	std::cout << "good\n";
	
	this->visit(ctx->ifThenBlock()->codeBlock());
	llvm::Value *thenValue = yupc::CompilationUnits.back()->ValueStack.top();

	yupc::CompilationUnits.back()->IRBuilder->CreateBr(mergeBB);
	thenBB = yupc::CompilationUnits.back()->IRBuilder->GetInsertBlock();

	func->getBasicBlockList().push_back(elseBB);
	yupc::CompilationUnits.back()->IRBuilder->SetInsertPoint(elseBB);

	this->visit(ctx->ifElseBlock()->codeBlock());
	llvm::Value *elseValue = yupc::CompilationUnits.back()->ValueStack.top();

	yupc::CompilationUnits.back()->IRBuilder->CreateBr(mergeBB);
	elseBB = yupc::CompilationUnits.back()->IRBuilder->GetInsertBlock();

	func->getBasicBlockList().push_back(mergeBB);
	yupc::CompilationUnits.back()->IRBuilder->SetInsertPoint(mergeBB);
	llvm::PHINode *pn = yupc::CompilationUnits.back()->IRBuilder->CreatePHI(
		llvm::Type::getDoubleTy(*yupc::CompilationUnits.back()->Context), 2, "");
	pn->addIncoming(thenValue, thenBB);
	pn->addIncoming(elseValue, elseBB);

	yupc::CompilationUnits.back()->ValueStack.push(pn);*/
	
	return nullptr;
}
