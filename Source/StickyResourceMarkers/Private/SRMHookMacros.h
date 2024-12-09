// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "SRMLogMacros.h"

#define BEGIN_BLUEPRINT_HOOK_DEFINITIONS \
    auto hookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>(); \
    int32 CurrentOffset = 0;

#define BEGIN_HOOK( CLASS, FUNCTION_NAME, OFFSET ) \
    CurrentOffset = OFFSET; \
    hookManager->HookBlueprintFunction( CLASS->FindFunctionByName(TEXT(#FUNCTION_NAME)), [&](FBlueprintHookHelper& helper) { \
        auto context = helper.GetContext(); \
        auto contextName = context->GetFName().GetPlainNameString(); \
        auto functionName = FString( #FUNCTION_NAME ); \
        SRM_LOG("%s::%s: %s (%s)" , *contextName, *functionName, *(OFFSET == EPredefinedHookOffset::Start ?  *FString("Start") : (OFFSET == EPredefinedHookOffset::Return ? *FString("Return") : FString::FromInt( OFFSET ))), *context->GetName() ); \

#define BEGIN_HOOK_START( CLASS, FUNCTION_NAME ) BEGIN_HOOK( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Start )
#define BEGIN_HOOK_RETURN( CLASS, FUNCTION_NAME ) BEGIN_HOOK( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Return )

#define LOG_HOOK_VARPTR( VARIABLE_NAME, HELPER_ACCESSOR, VARIABLE_ACCESSOR, PROPERTY_TYPE, FORMAT_STRING ) \
        auto VARIABLE_NAME##_helper = helper.HELPER_ACCESSOR(); \
        auto VARIABLE_NAME = VARIABLE_NAME##_helper->VARIABLE_ACCESSOR<PROPERTY_TYPE>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": "#FORMAT_STRING, *contextName, *functionName, *VARIABLE_NAME);

#define LOG_HOOK_INT( VARIABLE_NAME, HELPER_ACCESSOR ) \
        LOG_HOOK_VARPTR( VARIABLE_NAME, HELPER_ACCESSOR, GetVariablePtr, FIntProperty, %d )

#define LOG_LOCAL_INT( VARIABLE_NAME ) LOG_HOOK_INT( VARIABLE_NAME, GetLocalVariableHelper )
#define LOG_OUT_INT( VARIABLE_NAME ) LOG_HOOK_INT( VARIABLE_NAME, GetOutVariableHelper )
#define LOG_RETURN_INT LOG_OUT_INT( ReturnValue )

#define LOG_HOOK_BOOL( VARIABLE_NAME, HELPER_ACCESSOR ) \
        auto VARIABLE_NAME##_helper = helper.HELPER_ACCESSOR(); \
        auto VARIABLE_NAME = VARIABLE_NAME##_helper->GetBoolVariable(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": %s", *contextName, *functionName, VARIABLE_NAME ? TEXT("true") : TEXT("false") );

#define LOG_LOCAL_BOOL( VARIABLE_NAME ) LOG_HOOK_BOOL( VARIABLE_NAME, GetLocalVariableHelper )
#define LOG_OUT_BOOL( VARIABLE_NAME ) LOG_HOOK_BOOL( VARIABLE_NAME, GetOutVariableHelper )
#define LOG_RETURN_BOOL LOG_OUT_BOOL( ReturnValue )

#define LOG_HOOK_TEXT( VARIABLE_NAME, HELPER_ACCESSOR ) \
        auto VARIABLE_NAME##_helper = helper.HELPER_ACCESSOR(); \
        auto VARIABLE_NAME = VARIABLE_NAME##_helper->GetVariablePtr<FTextProperty>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": %s", *contextName, *functionName, *VARIABLE_NAME->ToString());

#define LOG_LOCAL_TEXT( VARIABLE_NAME ) LOG_HOOK_TEXT( VARIABLE_NAME, GetLocalVariableHelper )
#define LOG_OUT_TEXT( VARIABLE_NAME ) LOG_HOOK_TEXT( VARIABLE_NAME, GetOutVariableHelper )
#define LOG_RETURN_TEXT LOG_OUT_TEXT( ReturnValue )

#define LOG_HOOK_ENUM( VARIABLE_NAME, HELPER_ACCESSOR, ENUM_TYPE ) \
        auto VARIABLE_NAME##_helper = helper.HELPER_ACCESSOR(); \
        auto VARIABLE_NAME = VARIABLE_NAME##_helper->GetEnumVariablePtr<ENUM_TYPE>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME" ("#ENUM_TYPE"): %d", *contextName, *functionName, *VARIABLE_NAME);

#define LOG_LOCAL_ENUM( VARIABLE_NAME, ENUM_TYPE ) LOG_HOOK_ENUM( VARIABLE_NAME, GetLocalVariableHelper, ENUM_TYPE )
#define LOG_OUT_ENUM( VARIABLE_NAME, ENUM_TYPE ) LOG_HOOK_ENUM( VARIABLE_NAME, GetOutVariableHelper, ENUM_TYPE )
#define LOG_RETURN_ENUM( ENUM_TYPE ) LOG_OUT_ENUM( ReturnValue, ENUM_TYPE )

#define LOG_HOOK_OBJ( VARIABLE_NAME, HELPER_ACCESSOR ) \
        auto VARIABLE_NAME##_helper = helper.HELPER_ACCESSOR(); \
        auto VARIABLE_NAME = VARIABLE_NAME##_helper->GetVariablePtr<FObjectProperty>(TEXT(#VARIABLE_NAME));\
        auto VARIABLE_NAME##_fstring = FString::Printf( TEXT("%s::%s:\t"#VARIABLE_NAME), *contextName, *functionName ); \
        SRMDebugging::DumpUObject( VARIABLE_NAME##_fstring, VARIABLE_NAME->Get() );

#define LOG_CONTEXT_OBJ( VARIABLE_NAME ) LOG_HOOK_OBJ( VARIABLE_NAME, GetContextVariableHelper )
#define LOG_LOCAL_OBJ( VARIABLE_NAME ) LOG_HOOK_OBJ( VARIABLE_NAME, GetLocalVariableHelper )
#define LOG_OUT_OBJ( VARIABLE_NAME ) LOG_HOOK_OBJ( VARIABLE_NAME, GetOutVariableHelper )
#define LOG_RETURN_OBJ LOG_OUT_OBJ( ReturnValue )

#define FINISH_HOOK \
    }, CurrentOffset );

#define HOOK_OFFSET( CLASS, FUNCTION_NAME, OFFSET ) \
    BEGIN_HOOK( CLASS, FUNCTION_NAME, OFFSET ) \
    FINISH_HOOK

#define HOOK_START( CLASS, FUNCTION_NAME ) HOOK_OFFSET( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Start )
#define HOOK_RETURN( CLASS, FUNCTION_NAME ) HOOK_OFFSET( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Return )

#define HOOK_START_AND_RETURN( CLASS, FUNCTION_NAME ) \
    HOOK_START(CLASS, FUNCTION_NAME); \
    HOOK_RETURN(CLASS, FUNCTION_NAME);
