#include "proxy_url_extractor.h"
#include <fstream>
#include <vector>
#include "tokener.h"

namespace qh
{

    namespace {

        template< class _StringVector, 
        class StringType,
        class _DelimType> 
            inline void StringSplit(  
            const StringType& str, 
            const _DelimType& delims, 
            unsigned int maxSplits, 
            _StringVector& ret)
        {
            unsigned int numSplits = 0;

            // Use STL methods
            size_t start, pos;
            start = 0;

            do
            {
                pos = str.find_first_of( delims, start );

                if ( pos == start )
                {
                    ret.push_back(StringType());
                    start = pos + 1;
                }
                else if ( pos == StringType::npos || ( maxSplits && numSplits + 1== maxSplits ) )
                {
                    // Copy the rest of the string
                    ret.push_back(StringType());
                    *(ret.rbegin()) = StringType(str.data() + start, str.size() - start);
                    break;
                }
                else
                {
                    // Copy up to delimiter
                    //ret.push_back( str.substr( start, pos - start ) );
                    ret.push_back(StringType());
                    *(ret.rbegin()) = StringType(str.data() + start, pos - start);
                    start = pos + 1;
                }

                ++numSplits;

            }
            while ( pos != StringType::npos );
        }
    }

    ProxyURLExtractor::ProxyURLExtractor()
    {
    }

    bool ProxyURLExtractor::Initialize( const std::string& param_keys_path )
    {
        std::ifstream ifs;
        ifs.open(param_keys_path.data(), std::fstream::in);
        typedef std::vector<std::string> stringvector;
        stringvector keysvect;
        
        while (!ifs.eof()) {
            std::string line;
            getline(ifs, line);
            if (ifs.fail() && !ifs.eof()) {
                fprintf(stderr, "SubUrlExtractor::LoadParamKeysFile readfile_error=[%s] error!!", param_keys_path.data());
                ifs.close();
                return false;
            }
            if (line.empty()) continue;

            keysvect.clear();
            StringSplit(line, ",", static_cast<unsigned int>(-1), keysvect);
            assert(keysvect.size() >= 1);
            keys_set_.insert(keysvect.begin(), keysvect.end());
            keys_set_.erase("");
        }

        ifs.close();

        return true;
    }

    std::string ProxyURLExtractor::Extract( const std::string& raw_url )
    {
        std::string sub_url;
        ProxyURLExtractor::Extract(keys_set_, raw_url, sub_url);
        return sub_url;
    }

	void ProxyURLExtractor::Extract( const KeyItems& keys, const std::string& raw_url, std::string& sub_url )
	{
#if 1
		//TODO 请面试者在这里添加自己的代码实现以完成所需功能
		//其实原方法写的不错，就是有一些细节处理不到位样例集1通过，2局部不通过，我的核心思想是
		//尽量利用原来比较成熟的方法，将样本2中的一些钉子户进行剪枝成为样例1中比较标准类型


		std::string rawcp=raw_url;
		int i=0;
		int a[2]={0};  //a[0]放'='最后出现的位置,a[1]放'&'最后出现的位置

		//下面开始对原始字符串进行剪枝,使得第二组中一些特殊样例全部变成第一组中的标准样例
		//（因为原始方法已经能够通过第一组测试集）
		//***************这里定义标准样例为'='与'&'交替出现，且'='较早出现******************//
		//剪枝方法不仅能够排除异类，并且能够遍历一次的情况下解决问题


		while(i<rawcp.size())
		{

			if(rawcp[i]=='=')
			{
				a[0]=i;
			}
        /****************************以下的if条件为了解决下列情况，'='后面紧接一个'&'
		"http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&xxx&query=&yyy", ""
		这种情况下直接去掉'&query='这一段就能够同其他的情况做相同处理,变成
		"http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&xxx&yyy", ""
		******************************************************************************/
			if(i+1<rawcp.size()&&rawcp[i]=='='&&rawcp[i+1]=='&'&&a[1]>0)
			{
				rawcp=rawcp.substr(0,a[1])+rawcp.substr(i+1,rawcp.size()-i-1);
				i++;
				continue;
			}
       ////////////////////////////////////////////////////////////////////////////////////


		/****************************以下的if条件为了解决原有代码中不能够正确处理的
		"http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&xxx&query=http://hnujug.com/"
		根本原因在于上述这种类型不再是"&""="交替出现，所以这里做了一个剪枝，连续出现的两个"&"之间
		不会有关键信息，所以去掉"&xxx"这一段,变成
		"http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&query=http://hnujug.com/"
		******************************************************************************/
			if(rawcp[i]=='&'&&a[1]>a[0])
			{
				rawcp=rawcp.substr(0,a[1])+rawcp.substr(i,rawcp.size()-i);
				i++;
				continue;
			}
       ////////////////////////////////////////////////////////////////////////////////////
			if(rawcp[i]=='&')
			{
				a[1]=i;
			}

			i++;
		}

		/***************************下面的while循环意在解决这种情况：通过以上两道剪枝之后，还遗留下：
		"http://lk.brand.sogou.com/svc/r.php?&%23&url=%68ttp%3A//23.80.77.123/22/e/4", "%68ttp%3A//23.80.77.123/22/e/4"
		'&'第一次出现的位置比'='第一次出现位置早，那么直接减去第一次出现的'&'就成为标准的待处理url地址,变成
		"http://lk.brand.sogou.com/svc/r.php?%23&url=%68ttp%3A//23.80.77.123/22/e/4", "%68ttp%3A//23.80.77.123/22/e/4"
		*******************************************************************************/
		i=0;
		while(i<rawcp.size())
		{
			if(i+2<rawcp.size()&&rawcp[i]=='?'&&rawcp[i+1]=='&')
			{
				rawcp=rawcp.substr(0,i+1)+rawcp.substr(i+2,rawcp.size()-i-2);
				break;
			}
			i++;
		}
		/////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////  以上为我的剪枝
	    /////////////////////////////////////////////////////////////////////////


		Tokener token(rawcp);       // 输入的字符串不再是原始字符串，而是剪枝后的，剩下的都是原来处理方式
		token.skipTo('?');
		token.next(); //skip one char : '?' 
		std::string key;
		while (!token.isEnd()) {
			key = token.nextString('=');
			if (keys.find(key) != keys.end()) {
				const char* curpos = token.getCurReadPos();
				int nreadable = token.getReadableSize();

				/**
				* case 1: 
                *  raw_url="http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&a=http://hnujug.com/&xx=yy"
                *  sub_url="http://hnujug.com/"
                */
                sub_url = token.nextString('&');

                if (sub_url.empty() && nreadable > 0) {
                    /**
                    * case 2: 
                    * raw_url="http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&a=http://hnujug.com/"
                    * sub_url="http://hnujug.com/"
                    */
                    assert(curpos);
                    sub_url.assign(curpos, nreadable);
                }
            }
            token.skipTo('&');
            token.next();//skip one char : '&'
		}
#else
        //这是一份参考实现，但在特殊情况下工作不能符合预期
        Tokener token(raw_url);
        token.skipTo('?');
        token.next(); //skip one char : '?' 
        std::string key;
        while (!token.isEnd()) {
            key = token.nextString('=');
            if (keys.find(key) != keys.end()) {
                const char* curpos = token.getCurReadPos();
                int nreadable = token.getReadableSize();

                /**
                * case 1: 
                *  raw_url="http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&a=http://hnujug.com/&xx=yy"
                *  sub_url="http://hnujug.com/"
                */
                sub_url = token.nextString('&');

                if (sub_url.empty() && nreadable > 0) {
                    /**
                    * case 2: 
                    * raw_url="http://www.microsofttranslator.com/bv.aspx?from=&to=zh-chs&a=http://hnujug.com/"
                    * sub_url="http://hnujug.com/"
                    */
                    assert(curpos);
                    sub_url.assign(curpos, nreadable);
                }
            }
            token.skipTo('&');
            token.next();//skip one char : '&'
        }
#endif
    }

    std::string ProxyURLExtractor::Extract( const KeyItems& keys, const std::string& raw_url )
    {
        std::string sub_url;
        ProxyURLExtractor::Extract(keys, raw_url, sub_url);
        return sub_url;
    }
}
