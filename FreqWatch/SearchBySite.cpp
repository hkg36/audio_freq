#include "StdAfx.h"
#include "SearchBySite.h"


CSearchBySite::CSearchBySite(std::vector<FreqInfo> &freqdatatopost):freqlist(freqdatatopost)
{
}


CSearchBySite::~CSearchBySite(void)
{
}

bool CSearchBySite::StartSearch()
{
	songresult.clear();
	json2::CJsonObject mainobj;

	//mainobj.PutBool(L"procinfo",true);
	json2::PJsonArray dataarray=new json2::CJsonArray();
	mainobj.PutValue(L"data",dataarray);

	for(auto i=freqlist.begin();i!=freqlist.end();i++)
	{
		json2::PJsonObject onefreq=new json2::CJsonObject();
		dataarray->PutValue(onefreq);

		onefreq->Put(L"freq",i->freq);
		onefreq->Put(L"time",i->time);
	}

	GZipOutput strout;
	mainobj.Save(strout);
	strout.Flush();

	CSockAddr saddr(L"liveplustest.sinaapp.com",L"80");
	HTTPClient::SocketEx socket;

	HTTPClient::HttpRequest request("POST","/music/findSongByFreqData.php");
	request.setHost("liveplustest.sinaapp.com");
	request.setContentType("gzip/json");
	request.setContentLength(strout.memstream->GetBufferSize());
	if(0==socket.Connect(saddr.Addr(),saddr.Len()))
	{
		request.SendRequest(&socket);
		socket.Send((const char*)strout.memstream->GetBuffer(),strout.memstream->GetBufferSize());

		HTTPClient::HttpResponse response;
		if(response.RecvResponseHead(&socket))
		{
			CComPtr<IMemoryStream> memstream;
			CComQIPtr<IStream> outstream;
			ByteStream::CreateInstanse(&memstream);
			outstream=memstream;
			response.RecvResponseBody(&socket,outstream);

			json2::PJsonObject resobj;
			resobj=json2::CJsonReader::Prase((LPCSTR)memstream->GetBuffer(),(unsigned int)memstream->GetBufferSize());

			int server_errno=resobj->GetNumber(L"errno");
			CAtlString server_error=resobj->GetString(L"error");
			json2::PJsonArray songlist=resobj->Get(L"data");
			for(size_t i=0;i<songlist->Size();i++)
			{
				json2::PJsonObject song=songlist->Get(i);
				SongResult oneresult;
				oneresult.song_id=song->GetNumber(L"song_id");
				oneresult.match_count=song->GetNumber(L"count");
				oneresult.filename=song->GetString(L"filename");
				songresult.push_back(oneresult);
			}
		}
		else
		{
			CComPtr<IMemoryStream> memstream;
			CComQIPtr<IStream> outstream;
			ByteStream::CreateInstanse(&memstream);
			outstream=memstream;
			response.RecvResponseBody(&socket,outstream);
			return false;
		}
	}

	return true;
}