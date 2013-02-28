//===-- UseAuto/UseAuto.cpp - Use auto type specifier ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file provides the implementation of the UseAutoTransform class.
///
//===----------------------------------------------------------------------===//

#include "UseAuto.h"
#include "UseAutoActions.h"
#include "UseAutoMatchers.h"

using clang::ast_matchers::MatchFinder;
using namespace clang;
using namespace clang::tooling;

int UseAutoTransform::apply(const FileContentsByPath &InputStates,
                            RiskLevel MaxRisk,
                            const clang::tooling::CompilationDatabase &Database,
                            const std::vector<std::string> &SourcePaths,
                            FileContentsByPath &ResultStates) {
  RefactoringTool UseAutoTool(Database, SourcePaths);

  for (FileContentsByPath::const_iterator I = InputStates.begin(),
                                          E = InputStates.end();
       I != E; ++I)
    UseAutoTool.mapVirtualFile(I->first, I->second);

  unsigned AcceptedChanges = 0;

  MatchFinder Finder;
  UseAutoFixer Fixer(UseAutoTool.getReplacements(), AcceptedChanges, MaxRisk);

  Finder.addMatcher(makeIteratorMatcher(), &Fixer);

  if (int Result = UseAutoTool.run(newFrontendActionFactory(&Finder))) {
    llvm::errs() << "Error encountered during translation.\n";
    return Result;
  }

  RewriterContainer Rewrite(UseAutoTool.getFiles(), InputStates);

  // FIXME: Do something if some replacements didn't get applied?
  UseAutoTool.applyAllReplacements(Rewrite.getRewriter());

  collectResults(Rewrite.getRewriter(), InputStates, ResultStates);

  if (AcceptedChanges > 0)
    setChangesMade();

  return 0;
}
