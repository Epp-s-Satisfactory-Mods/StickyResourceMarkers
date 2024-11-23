// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"

#define BEGIN_BLUEPRINT_HOOK_DEFINITIONS \
    auto hookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>(); \
    int32 CurrentOffset = 0;

#define BEGIN_HOOK( CLASS, FUNCTION_NAME, OFFSET ) \
    CurrentOffset = OFFSET; \
    hookManager->HookBlueprintFunction( CLASS->FindFunctionByName(TEXT(#FUNCTION_NAME)), [&](FBlueprintHookHelper& helper) { \
        auto context = helper.GetContext(); \
        auto contextName = context->GetFName().GetPlainNameString(); \
        auto functionName = FString( #FUNCTION_NAME ); \
        SRM_LOG("%s::%s: %s (%s)" , *contextName, *functionName, *(OFFSET == EPredefinedHookOffset::Start ? L"Start" : (OFFSET == EPredefinedHookOffset::Return ? L"Return" : FString::FromInt( OFFSET ))), *context->GetName() ); \

#define BEGIN_HOOK_START( CLASS, FUNCTION_NAME ) BEGIN_HOOK( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Start )
#define BEGIN_HOOK_RETURN( CLASS, FUNCTION_NAME ) BEGIN_HOOK( CLASS, FUNCTION_NAME, EPredefinedHookOffset::Return )

#define LOG_HOOK_INT( VARIABLE_NAME, HELPER_FUNC ) \
        auto VARIABLE_NAME = helper.HELPER_FUNC<FIntProperty>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": %d", *contextName, *functionName, *VARIABLE_NAME);

#define LOG_LOCAL_INT( VARIABLE_NAME ) LOG_HOOK_INT( VARIABLE_NAME, GetLocalVarPtr )
#define LOG_OUT_INT( VARIABLE_NAME ) LOG_HOOK_INT( VARIABLE_NAME, GetOutVariablePtr )
#define LOG_RETURN_INT LOG_OUT_INT( ReturnValue )

#define LOG_HOOK_BOOL( VARIABLE_NAME, HELPER_FUNC ) \
        auto VARIABLE_NAME = helper.HELPER_FUNC(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": %s", *contextName, *functionName, VARIABLE_NAME ? TEXT("true") : TEXT("false") );

#define LOG_LOCAL_BOOL( VARIABLE_NAME ) LOG_HOOK_BOOL( VARIABLE_NAME, GetLocalVarBool )
#define LOG_OUT_BOOL( VARIABLE_NAME ) LOG_HOOK_BOOL( VARIABLE_NAME, GetOutVarBool )
#define LOG_RETURN_BOOL LOG_OUT_BOOL( ReturnValue )

#define LOG_HOOK_TEXT( VARIABLE_NAME, HELPER_FUNC ) \
        auto VARIABLE_NAME = helper.HELPER_FUNC<FTextProperty>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME": \"%s\"", *contextName, *functionName, *VARIABLE_NAME->ToString());

#define LOG_LOCAL_TEXT( VARIABLE_NAME ) LOG_HOOK_TEXT( VARIABLE_NAME, GetLocalVarPtr )
#define LOG_OUT_TEXT( VARIABLE_NAME ) LOG_HOOK_TEXT( VARIABLE_NAME, GetOutVariablePtr )
#define LOG_RETURN_TEXT LOG_OUT_TEXT( ReturnValue )

#define LOG_HOOK_ENUM( VARIABLE_NAME, HELPER_FUNC, ENUM_TYPE ) \
        auto VARIABLE_NAME = helper.HELPER_FUNC<ENUM_TYPE>(TEXT(#VARIABLE_NAME)); \
        SRM_LOG("%s::%s:\t"#VARIABLE_NAME" ("#ENUM_TYPE"): %d", *contextName, *functionName, *VARIABLE_NAME);

#define LOG_LOCAL_ENUM( VARIABLE_NAME, ENUM_TYPE ) LOG_HOOK_ENUM( VARIABLE_NAME, GetLocalVarEnumPtr, ENUM_TYPE )
#define LOG_OUT_ENUM( VARIABLE_NAME, ENUM_TYPE ) LOG_HOOK_ENUM( VARIABLE_NAME, GetOutVarEnumPtr, ENUM_TYPE )
#define LOG_RETURN_ENUM( ENUM_TYPE ) LOG_OUT_ENUM( ReturnValue, ENUM_TYPE )

//#define LOG_HOOK_VARIABLE( VARIABLE_NAME, HELPER_FUNC, FPROPERTY_TYPE, DUMPFUNC ) \
//        auto VARIABLE_NAME = helper.HELPER_FUNC<FPROPERTY_TYPE>(TEXT(#VARIABLE_NAME));\
//        auto VARIABLE_NAME##_fstring FString::Printf( TEXT("%s::%s:\t"#VARIABLE_NAME), *contextName, *functionName ) \
//        DUMPFUNC( VARIABLE_NAME##_fstring, VARIABLE_NAME );

#define LOG_HOOK_OBJ( VARIABLE_NAME, HELPER_FUNC ) \
        auto VARIABLE_NAME = helper.HELPER_FUNC<FObjectProperty>(TEXT(#VARIABLE_NAME));\
        auto VARIABLE_NAME##_fstring = FString::Printf( TEXT("%s::%s:\t"#VARIABLE_NAME), *contextName, *functionName ); \
        DumpObject( VARIABLE_NAME##_fstring, VARIABLE_NAME->Get() );

#define LOG_LOCAL_OBJ( VARIABLE_NAME ) LOG_HOOK_OBJ( VARIABLE_NAME, GetLocalVarPtr )
#define LOG_OUT_OBJ( VARIABLE_NAME ) LOG_HOOK_OBJ( VARIABLE_NAME, GetOutVariablePtr )
#define LOG_RETURN_OBJ LOG_OUT_OBJ( ReturnValue )

#define LOG_CONTEXT_PROPERTY( PROPERTY_NAME, FPROPERTY_TYPE, DUMPFUNC ) \
        auto PROPERTY_NAME = helper.GetContextVarPtr<FPROPERTY_TYPE>(TEXT(#PROPERTY_NAME));\
        FString PROPERTY_NAME##_fstring = FString::Printf( TEXT("%s::%s:\t"#PROPERTY_NAME), *contextName, *functionName );\
        DUMPFUNC( PROPERTY_NAME##_fstring, PROPERTY_NAME );

#define LOG_CONTEXT_OBJ( PROPERTY_NAME ) \
        auto PROPERTY_NAME = helper.GetContextVarPtr<FObjectProperty>(TEXT(#PROPERTY_NAME));\
        FString PROPERTY_NAME##_fstring = FString::Printf( TEXT("%s::%s:\t"#PROPERTY_NAME), *contextName, *functionName );\
        DumpObject( PROPERTY_NAME##_fstring, PROPERTY_NAME->Get() );

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
