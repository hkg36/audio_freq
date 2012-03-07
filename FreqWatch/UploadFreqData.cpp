#include "StdAfx.h"
#include "UploadFreqData.h"


CUploadFreqData::CUploadFreqData(void)
{
	db.Open(L"D:\\freq_info.data.db");
	int res=db.Execute(L"CREATE TABLE WorkOption (name  TEXT(64),value  INTEGER,PRIMARY KEY (name))");
	readOption=db.Prepare(L"select value from WorkOption where name=?1");
	writeOption=db.Prepare(L"replace into WorkOption(name,value) values(?1,?2)");
	getNextSongToUpload=db.Prepare(L"select id,name from songlist where id>?1 order by id limit 1");
	getAnchorPoint=db.Prepare(L"select id,freq,time from Anchor_freq_index where song_id=?1");
	getChecker=db.Prepare(L"select freq,time_offset from Check_freq_index where Anchor_id=?1");
}


CUploadFreqData::~CUploadFreqData(void)
{
	readOption.Close();
	writeOption.Close();
	getNextSongToUpload.Close();
	getAnchorPoint.Close();
	getChecker.Close();
	db.Close();
}
struct CheckInfo
{
	int freq,time_offset;
};
struct AnchorInfo
{
	int id,freq,time;
	std::vector<CheckInfo> checkinfos;
};
int CUploadFreqData::DoUpload()
{
	int songid=0;
	CAtlString songname;
	readOption.Bind(1,L"lastUploadSong");
	if(readOption.Step()==SQLITE_ROW)
	{
		songid=readOption.GetInt(0);
	}
	readOption.Reset();

	getNextSongToUpload.Bind(1,songid);
	if(getNextSongToUpload.Step()==SQLITE_ROW)
	{
		songid=getNextSongToUpload.GetInt(0);
		songname=CA2W(getNextSongToUpload.GetText(1),CP_UTF8);
	}
	getNextSongToUpload.Reset();

	std::vector<AnchorInfo> anchorlist;
	getAnchorPoint.Bind(1,songid);
	while(getAnchorPoint.Step()==SQLITE_ROW)
	{
		AnchorInfo info;
		info.id=getAnchorPoint.GetInt(0);
		info.freq=getAnchorPoint.GetInt(1);
		info.time=getAnchorPoint.GetInt(2);

		anchorlist.push_back(std::move(info));
	}
	getAnchorPoint.Reset();

	for(auto i=anchorlist.begin();i!=anchorlist.end();++i)
	{
		getChecker.Bind(1,i->id);
		while(SQLITE_ROW==getChecker.Step())
		{
			CheckInfo cinfo;
			cinfo.freq=getChecker.GetInt(0);
			cinfo.time_offset=getChecker.GetInt(1);
			i->checkinfos.push_back(cinfo);
		}
		getChecker.Reset();
	}

	json2::CJsonObject mainobj;
	mainobj.Put(L"song",songname);
	json2::PJsonArray AnchorList=new json2::CJsonArray();
	mainobj.PutValue(L"anchors",AnchorList);

	for(auto i=anchorlist.begin();i!=anchorlist.end();++i)
	{
		json2::PJsonObject anchorObject=new json2::CJsonObject();
		AnchorList->PutValue(anchorObject);
		anchorObject->Put(L"freq",i->freq);
		anchorObject->Put(L"time",i->time);

		json2::PJsonArray checkArray=new json2::CJsonArray();
		anchorObject->PutValue(L"checks",checkArray);
		for(auto j=i->checkinfos.begin();j!=i->checkinfos.end();++j)
		{
			json2::PJsonObject checker=new json2::CJsonObject();
			checkArray->PutValue(checker);

			checker->Put(L"freq",j->freq);
			checker->Put(L"offset",j->time_offset);
		}
	}

	StringOutput strout;
	mainobj.Save(strout);
	return 0;
}