/*******************************************************************************
 *  @file      UrlScan.h 2013\10\10 17:58:33 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef URLSCAN_4DCE0B12_C454_4B1F_B574_1FF778D9FD99_H__
#define URLSCAN_4DCE0B12_C454_4B1F_B574_1FF778D9FD99_H__

#include <string>

/******************************************************************************/
struct ICBUrlScanner;
/**
 * The class <code>UrlScan</code> 
 *
 */
class UrlScanner
{
public:
    typedef wchar_t CHAR;
    typedef std::basic_string<CHAR> STRING;
    struct MatchNode 
    {
        short brother, child;
        CHAR ch;
    };
    enum 
    {
        ST_INIT = 0, 
        ST_URLSEP, 
        ST_SKIP, 
        ST_HEADER, 
        ST_BODYBEGIN, 
        ST_BODY
    };

public:
    void init(ICBUrlScanner* pCB,int offset);
    int feed(CHAR ch);
    int flush();
    inline bool IsUrlSeps(CHAR ch)
    {
        return (ch>255||ch<0)?true:urlseps_[ch];
    }
    bool IsUrlCur()
    {
        return (stat_ == ST_HEADER ||
            stat_ == ST_BODYBEGIN ||
            stat_ == ST_BODY);
    }

private:
    inline bool IsUrlLeading(CHAR ch)
    {
        return (ch>255||ch<0)?false:leading_[ch];
    }
    ICBUrlScanner* pCB_;

    STRING              url_;
    int                 stat_;
    const MatchNode*    pNode_;
    const static MatchNode matchTree_[];
    const static bool   leading_[], urlseps_[];
    int     offset_;
    unsigned urlBegin_, counter_;
};
struct ICBUrlScanner 
{
    virtual int _outputUrlCallback(unsigned pos, const UrlScanner::STRING& url) = 0;
};
/******************************************************************************/
#endif// URLSCAN_4DCE0B12_C454_4B1F_B574_1FF778D9FD99_H__
