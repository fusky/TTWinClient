/******************************************************************************* 
 *  @file      GlobalMacros.h 2012\3\18 14:11:38 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @summary   
/******************************************************************************/

#ifndef CODEPATHCONTROL_24539E02_F185_48E3_A24F_280CCB8B3EDA_H__
#define CODEPATHCONTROL_24539E02_F185_48E3_A24F_280CCB8B3EDA_H__

/******************************************************************************/

#define CHECK_BOOL(exp)														\
    do {																	\
        if (!(exp))															\
        {																	\
            goto END0;														\
        }																	\
    } while(0)

#define ERROR_CHECK_BOOL(exp)												\
    do {																	\
        if (!(exp))															\
        {																	\
            goto END0;														\
        }																	\
    } while(0)

#define CHECK_BOOLEX(exp, exp1)												\
    do {																	\
        if (!(exp))															\
        {																	\
            exp1;															\
            goto END0;														\
        }																	\
    } while(0)

#define ERROR_CHECK_BOOLEX(exp, exp1)										\
    do {																	\
        if (!(exp))															\
        {																	\
            exp1;															\
            goto END0;														\
        }																	\
    } while(0)

#define CHECK_COM(exp)														\
    do {																	\
        if (!SUCCEEDED(exp))												\
        {																	\
            goto END0;														\
        }																	\
    } while(0)

#define ERROR_CHECK_COM(exp)												\
    do {																	\
        if (!SUCCEEDED(exp))												\
        {																	\
            goto END0;														\
        }																	\
    } while(0)

#define ERROR_CHECK_COMEX(exp, exp1)										\
    do {																	\
        if (!SUCCEEDED(exp))												\
        {																	\
            exp1;															\
            goto END0;														\
        }																	\
    } while(0)

#define QUIT()          \
    do                  \
    {                   \
        goto END0;      \
    } while (0)

/******************************************************************************/
#endif // CODEPATHCONTROL_24539E02_F185_48E3_A24F_280CCB8B3EDA_H__