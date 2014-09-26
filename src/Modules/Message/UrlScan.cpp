/******************************************************************************* 
 *  @file      UrlScan.cpp 2013\10\10 17:58:35 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "UrlScan.h"

/******************************************************************************/

/************** M-TREE generate ***********************/
const bool UrlScanner::urlseps_[]={
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 
};
const bool UrlScanner::leading_[]={
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};
const UrlScanner::MatchNode UrlScanner::matchTree_[] = {
    {  11,    1, 'f'}, //   0
    {   5,    2, 'i'}, //   1
    {  -1,    3, 'l'}, //   2
    {  -1,    4, 'e'}, //   3
    {  -1,   -1, ':'}, //   4
    {  -1,    6, 't'}, //   5
    {  -1,    7, 'p'}, //   6
    {   8,   -1, '.'}, //   7
    {   9,   -1, ':'}, //   8
    {  -1,   10, 's'}, //   9
    {  -1,   -1, ':'}, //  10
    {  18,   12, 'g'}, //  11
    {  -1,   13, 'o'}, //  12
    {  -1,   14, 'p'}, //  13
    {  -1,   15, 'h'}, //  14
    {  -1,   16, 'e'}, //  15
    {  -1,   17, 'r'}, //  16
    {  -1,   -1, ':'}, //  17
    {  25,   19, 'h'}, //  18
    {  -1,   20, 't'}, //  19
    {  -1,   21, 't'}, //  20
    {  -1,   22, 'p'}, //  21
    {  23,   -1, ':'}, //  22
    {  -1,   24, 's'}, //  23
    {  -1,   -1, ':'}, //  24
    {  37,   26, 'm'}, //  25
    {  32,   27, 'a'}, //  26
    {  -1,   28, 'i'}, //  27
    {  -1,   29, 'l'}, //  28
    {  -1,   30, 't'}, //  29
    {  -1,   31, 'o'}, //  30
    {  -1,   -1, ':'}, //  31
    {  -1,   33, 'm'}, //  32
    {  -1,   34, 's'}, //  33
    {  35,   -1, ':'}, //  34
    {  -1,   36, 't'}, //  35
    {  -1,   -1, ':'}, //  36
    {  46,   38, 'n'}, //  37
    {  42,   39, 'e'}, //  38
    {  -1,   40, 'w'}, //  39
    {  -1,   41, 's'}, //  40
    {  -1,   -1, ':'}, //  41
    {  -1,   43, 'n'}, //  42
    {  -1,   44, 't'}, //  43
    {  -1,   45, 'p'}, //  44
    {  -1,   -1, ':'}, //  45
    {  55,   47, 'p'}, //  46
    {  -1,   48, 'r'}, //  47
    {  -1,   49, 'o'}, //  48
    {  -1,   50, 's'}, //  49
    {  -1,   51, 'p'}, //  50
    {  -1,   52, 'e'}, //  51
    {  -1,   53, 'r'}, //  52
    {  -1,   54, 'o'}, //  53
    {  -1,   -1, ':'}, //  54
    {  60,   56, 'r'}, //  55
    {  -1,   57, 't'}, //  56
    {  -1,   58, 's'}, //  57
    {  -1,   59, 'p'}, //  58
    {  -1,   -1, ':'}, //  59
    {  67,   61, 't'}, //  60
    {  -1,   62, 'e'}, //  61
    {  -1,   63, 'l'}, //  62
    {  -1,   64, 'n'}, //  63
    {  -1,   65, 'e'}, //  64
    {  -1,   66, 't'}, //  65
    {  -1,   -1, ':'}, //  66
    {  -1,   68, 'w'}, //  67
    {  72,   69, 'a'}, //  68
    {  -1,   70, 'i'}, //  69
    {  -1,   71, 's'}, //  70
    {  -1,   -1, ':'}, //  71
    {  -1,   73, 'w'}, //  72
    {  -1,   74, 'w'}, //  73
    {  -1,   -1, '.'}  //  74
};

/************** M-TREE generate ***********************/

void UrlScanner::init(ICBUrlScanner* pCB,int offset)
{
    pCB_ = pCB;
    counter_ = urlBegin_ = 0;
    url_ = STRING();
    stat_ = ST_INIT;
    pNode_ = &matchTree_[0];
    offset_ = offset;
}

#define BeginURLHeader { \
    pNode_ = &matchTree_[0]; url_ = STRING(); stat_ = ST_HEADER; \
    goto _begin_header; \
}
int UrlScanner::feed(UrlScanner::CHAR ch)
{
    int c0;

    ++counter_;
    switch (stat_)
    {
	    case ST_INIT:
		    if (IsUrlLeading(ch))
				BeginURLHeader;
			if (IsUrlSeps(ch)) 
                stat_ = ST_URLSEP;
			else 
                stat_ = ST_INIT;
			break;
		case ST_URLSEP:
			if (IsUrlSeps(ch)) 
                break;
			if (IsUrlLeading(ch))
			{
				BeginURLHeader;
			}
			else 
                stat_ = ST_SKIP;
			break;
		case ST_SKIP:
			if (IsUrlSeps(ch))
				stat_ = ST_URLSEP; //just skip it
			break;
		case ST_HEADER:
_begin_header:                      //matching header
            c0 = tolower(ch);
            while(pNode_->ch != c0)
            {
	            if (pNode_->brother < 0)
	            {
                    if (IsUrlLeading(c0))
                        BeginURLHeader;
                    //mismatch
                    stat_ = ST_INIT;
                    return 0;//just skip it
                }
                pNode_ = &matchTree_[pNode_->brother];
            }
	        //matched, add to url
	        if (url_.empty()) 
                urlBegin_ = counter_-1; //first TCHAR
	        url_.append(1, (CHAR)ch);
	        //step to next
	        if (pNode_->child < 0) 
		        stat_=ST_BODYBEGIN;
	        else 
		        pNode_ = &matchTree_[pNode_->child];
	        break;
        case ST_BODYBEGIN:
	        if (IsUrlSeps(ch)) 
		        stat_ = ST_URLSEP; //just skip it
	        else
	        {
		        url_.append(1, (CHAR)ch);
		        stat_ = ST_BODY;
	        }
	        break;
        case ST_BODY:
	        if (IsUrlSeps(ch))
	        {
		        //url end, skip ch
		        stat_ = ST_URLSEP;
		        return pCB_->_outputUrlCallback(urlBegin_+offset_, url_);
	        }
	        else
		        url_.append(1, (CHAR)ch);
	        break;
        default:
	        break;
    }
    return 0;
}
int UrlScanner::flush()
{
    int ret=0;
    if (stat_ == ST_BODY)
	    ret=pCB_->_outputUrlCallback(urlBegin_+offset_, url_);
    return ret;
}

/******************************************************************************/