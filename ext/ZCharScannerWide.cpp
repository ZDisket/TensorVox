#include "ZCharScannerWide.h"
using namespace std;
#include <stdexcept>

int ZStringDelimiterWide::key_search(const std::wstring& s, const std::wstring& key)
{
	int count = 0;
	size_t pos = 0;
    while ((pos = s.find(key, pos)) != std::wstring::npos) {
		++count;
		++pos;
	}
	return count;
}
void ZStringDelimiterWide::UpdateTokens()
{
    if (!m_vDelimiters.size() || m_sString == L"")
		return;

	m_vTokens.clear();


    vector<std::wstring>::iterator dIt = m_vDelimiters.begin();
	while (dIt != m_vDelimiters.end())
	{
        std::wstring delimiter = *dIt;
	

		DelimStr(m_sString, delimiter, true);
		
	
		++dIt;
	}
	
	

}


void ZStringDelimiterWide::DelimStr(const std::wstring & s, const std::wstring & delimiter, const bool & removeEmptyEntries)
{
	BarRange(0, s.length());
	for (size_t start = 0, end; start < s.length(); start = end + delimiter.length())
	{
		size_t position = s.find(delimiter, start);
        end = position != std::wstring::npos ? position : s.length();

        std::wstring token = s.substr(start, end - start);
		if (!removeEmptyEntries || !token.empty())
		{
			if (token != s)
				m_vTokens.push_back(token);

		}
		Bar(position);
	}

	// dadwwdawdaawdwadwd
}

void ZStringDelimiterWide::BarRange(const int & min, const int & max)
{
#ifdef _AFX_ALL_WARNINGS
	if (PgBar)
		m_pBar->SetRange32(min, max);


#endif
}

void ZStringDelimiterWide::Bar(const int & pos)
{
#ifdef _AFX_ALL_WARNINGS
	if (PgBar)
		m_pBar->SetPos(pos);


#endif
}

ZStringDelimiterWide::ZStringDelimiterWide()
{
    m_sString = L"";
	tokenIndex = 0;
	PgBar = false;
}


bool ZStringDelimiterWide::GetFirstToken(std::wstring & in_out)
{
	if (m_vTokens.size() >= 1) {
		in_out = m_vTokens[0];
		return true;
	}
	else {
        return false;
	}
}

bool ZStringDelimiterWide::GetNextToken(std::wstring & in_sOut)
{
	if (tokenIndex > m_vTokens.size() - 1)
		return false;

	in_sOut = m_vTokens[tokenIndex];
	++tokenIndex;

	return true;
}

std::wstring ZStringDelimiterWide::operator[](const size_t & in_index)
{
	if (in_index > m_vTokens.size())
		throw std::out_of_range("ZStringDelimiter tried to access token higher than size");

	return m_vTokens[in_index];

}
std::wstring ZStringDelimiterWide::Reassemble(const std::wstring& delim, const int& nelem)
{
    std::wstring Result = L"";
	TokenIterator RasIt = m_vTokens.begin();
	int r = 0;
	if (nelem == -1) {
		while (RasIt != m_vTokens.end())
		{

			if (r != 0)
				Result.append(delim);

			Result.append(*RasIt);

			++r;


			++RasIt;
		}
	}
	else {
		while (RasIt != m_vTokens.end() && r < nelem)
		{
		
			if (r != 0)
				Result.append(delim);

			Result.append(*RasIt);

			++r;
			++RasIt;
		}
	}
	
	return Result;

}

std::wstring ZStringDelimiterWide::Reassemble(const std::wstring & delim, const std::vector<std::wstring>& Strs,int nelem)
{
    std::wstring Result = L"";
	TokenIterator RasIt = Strs.begin();
	int r = 0;
	if (nelem == -1) {
		while (RasIt != Strs.end())
		{

			if (r != 0)
				Result.append(delim);

			Result.append(*RasIt);

			++r;


			++RasIt;
		}
	}
	else {
		while (RasIt != Strs.end() && r < nelem)
		{

			if (r != 0)
				Result.append(delim);

			Result.append(*RasIt);

			++r;
			++RasIt;
		}
	}

	return Result;
}

void ZStringDelimiterWide::AddDelimiter(const std::wstring & in_Delim)
{
	m_vDelimiters.push_back(in_Delim);
	UpdateTokens();

}

void ZStringDelimiterWide::SetDelimiters(const std::vector<std::wstring> &Delims)
{
    m_vDelimiters.assign(Delims.begin(),Delims.end());
    UpdateTokens();

}

ZStringDelimiterWide::~ZStringDelimiterWide()
{
}
