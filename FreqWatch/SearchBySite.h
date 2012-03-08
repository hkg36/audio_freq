#pragma once

class CSearchBySite
{
	std::vector<FreqInfo> freqlist;
public:
	CSearchBySite(std::vector<FreqInfo> &freqdatatopost);
	~CSearchBySite(void);

	bool StartSearch();
};

