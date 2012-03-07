#pragma once
class CUploadFreqData
{
private:
	CSqlite db;
	CSqliteStmt readOption;
	CSqliteStmt writeOption;
	CSqliteStmt getNextSongToUpload;
	CSqliteStmt getAnchorPoint;
	CSqliteStmt getChecker;
public:
	CUploadFreqData(void);
	~CUploadFreqData(void);

	int DoUpload();
};

