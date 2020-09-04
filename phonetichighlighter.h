#ifndef PHONETICHIGHLIGHTER_H
#define PHONETICHIGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QRegularExpression>
class PhoneticHighlighter : public QSyntaxHighlighter
{
public:
    PhoneticHighlighter(QTextDocument *parent = 0);

    // This is public because the main window uses it
    QRegularExpression PhonemeExp;


protected:
    void highlightBlock(const QString &text) override;
private:
    QTextCharFormat PhonemeFormat;

};

#endif // PHONETICHIGHLIGHTER_H
