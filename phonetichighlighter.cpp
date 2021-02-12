#include "phonetichighlighter.h"


PhoneticHighlighter::PhoneticHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{

    QString MatchExp = "\\{(\\s*?.*?)*?\\}";
    PhonemeFormat.setForeground(Qt::magenta);
    PhonemeFormat.setFontWeight(QFont::Bold);
    PhonemeExp = QRegularExpression(MatchExp);

    QString SingleExp = "@.\\S*";
    SinglePhonemeExp = QRegularExpression(SingleExp);

    QString LongExp = "\\b\\w{23,}";
    TooLongExp = QRegularExpression(LongExp);

    ErrorFormat = PhonemeFormat;
    ErrorFormat.setForeground(Qt::red);
    ErrorFormat.setBackground(Qt::black);






}

void PhoneticHighlighter::highlightBlock(const QString &text)
{

    // Phoneme
    HighlightRegex(text,PhonemeExp,PhonemeFormat);
    HighlightRegex(text,SinglePhonemeExp,PhonemeFormat);

    // Error
    HighlightRegex(text,TooLongExp,ErrorFormat);

}

void PhoneticHighlighter::HighlightRegex(const QString& Text,const QRegularExpression &Reg, const QTextCharFormat &Fmt)
{
    QRegularExpressionMatchIterator MatchIter = Reg.globalMatch(Text);
    while (MatchIter.hasNext()) {
        QRegularExpressionMatch match = MatchIter.next();
        setFormat(match.capturedStart(), match.capturedLength(), Fmt);
    }

}
