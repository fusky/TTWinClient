/*******************************************************************************
 *  @file      TTLogic_Dll.h 2012\9\11 10:49:01 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef LOGICENGINEDLL_C52F701C_82BF_4A54_BAED_A106C4DF6CCB_H__
#define LOGICENGINEDLL_C52F701C_82BF_4A54_BAED_A106C4DF6CCB_H__

/******************************************************************************/

#ifndef TTLOGIC_DLL
	#define TTLOGIC_API			__declspec( dllimport )
	#define TTLOGIC_DATA		__declspec( dllimport )
	#define TTLOGIC_CLASS		__declspec( dllimport )
#else
	#define TTLOGIC_API			__declspec( dllexport )
	#define TTLOGIC_DATA		__declspec( dllexport )
	#define TTLOGIC_CLASS		__declspec( dllexport )
#endif

/******************************************************************************/
#endif// LOGICENGINEDLL_C52F701C_82BF_4A54_BAED_A106C4DF6CCB_H__