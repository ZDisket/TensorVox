#include "phonetichighlighter.h"


PhoneticHighlighter::PhoneticHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{

    QString MatchExp = "\\{(\\s*?.*?)*?\\}";
    PhonemeFormat.setForeground(Qt::magenta);
    PhonemeFormat.setFontWeight(QFont::Bold);
    PhonemeExp = QRegularExpression(MatchExp);



}

void PhoneticHighlighter::highlightBlock(const QString &text)
{
    QRegularExpressionMatchIterator MatchIter = PhonemeExp.globalMatch(text);
    while (MatchIter.hasNext()) {
        QRegularExpressionMatch match = MatchIter.next();
        setFormat(match.capturedStart(), match.capturedLength(), PhonemeFormat);
    }

}
