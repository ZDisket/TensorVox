#pragma once

#define GBasicCharScanner ZStringDelimiter

#include <vector>
#include <string>

// We need ZCharScanner but for wstrings. I copy class, fastest way
typedef std::vector<std::wstring>::const_iterator TokenIterator;

// ZStringDelimiter
// ==============
// Simple class to delimit and split strings.
// You can use operator[] to access them
// Or you can use the itBegin() and itEnd() to get some iterators
// =================
class ZStringDelimiter
{
private:
    int key_search(const std::wstring & s, const std::wstring & key);
	void UpdateTokens();
    std::vector<std::wstring> m_vTokens;
    std::vector<std::wstring> m_vDelimiters;

    std::wstring m_sString;

    void DelimStr(const std::wstring& s, const std::wstring& delimiter, const bool& removeEmptyEntries = false);
	void BarRange(const int& min, const int& max);
	void Bar(const int& pos);
	size_t tokenIndex;
public:
	ZStringDelimiter();
	bool PgBar;

#ifdef _AFX_ALL_WARNINGS
	CProgressCtrl* m_pBar;
#endif

    ZStringDelimiter(const std::wstring& in_iStr) {
		m_sString = in_iStr;
		PgBar = false;

	}

    bool GetFirstToken(std::wstring& in_out);
    bool GetNextToken(std::wstring& in_sOut);

	// std::String alts

	size_t szTokens() { return m_vTokens.size(); }
    std::wstring operator[](const size_t& in_index);

    std::wstring Reassemble(const std::wstring & delim, const int & nelem = -1);

	// Override to reassemble provided tokens.
    std::wstring Reassemble(const std::wstring & delim, const std::vector<std::wstring>& Strs,int nelem = -1);

	// Get a const reference to the tokens
    const std::vector<std::wstring>& GetTokens() { return m_vTokens; }

	TokenIterator itBegin() { return m_vTokens.begin(); }
	TokenIterator itEnd() { return m_vTokens.end(); }

    void SetText(const std::wstring& in_Txt) {
		m_sString = in_Txt; 
		if (m_vDelimiters.size())
			UpdateTokens();
	}
    void AddDelimiter(const std::wstring& in_Delim);
    void SetDelimiters(const std::vector<std::wstring>& Delims);

	~ZStringDelimiter();
};

