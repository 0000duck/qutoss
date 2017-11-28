#include "equationparser.h"

EquationParser::EquationParser()
{
    ae_open();
}

EquationParser::~EquationParser()
{
    ae_close();
}

// the purpose of this function is to modify the input so that it is interpretable by lua
string EquationParser::standardize(string raw)
{
    if (raw.length() == 0)
        return "";

    string altered = raw;
    original = raw;
    string operators = "/*^+-";
    vector<string> funcNames = {"sin", "cos", "tan", "exp", "sinh", "cosh", "tanh"};

    // remove all spaces
    for (int i = 0; i < altered.size(); ++i)
        if (altered[i] == ' ')
        {
            altered.erase(i, 1);
            --i;
        }

    // replace all occurrences of e^ with exp
    for (int i = 0; i < altered.size() - 1; ++i)
        if (altered[i] == 'e' && altered[i+1] == '^')
             altered.replace(altered.begin() + i, altered.begin() + i + 2, "exp");

    // replace all occurrences of r (and not part of sqrt) with sqrt(xx + yy)
    for (int i = 0; i < altered.size(); ++i)
        if (altered[i] == 'r')
            if (i == altered.size() - 1 || altered[i+1] != 't')  // crude check for isolation
                altered.replace(altered.begin() + i, altered.begin() + i + 1, "sqrt(xx + yy)");

    // insert brackets, i.e sinxy => sin(xy)
    // do this from right to left (innermost first)
    for (int i = altered.size()-1; i >= 0; --i)
    {
        for (int id = 0; id < funcNames.size(); ++id)
        {
            string target = funcNames[id];
            int k = i;
            bool matched = false;
            while (k < altered.size() && !matched)
            {
                if (altered[k] == target[k - i])
                    ++k;
                else
                    break;
                if (k - i == target.size() && altered[k] != 'h')  // cosh, tanh, sinh
                    matched = true;
            }
            if (matched)
            {
                // if no brackets added, we do it automatically
                bool autoBracketed = false;
                if (altered[k] != '(')
                {
                    autoBracketed = true;
                    altered.insert(altered.begin() + k, '(');
                }

                int j = k+1;
                bool metOperator = false;
                int unclosedBrackets = 0;

                while (!metOperator && j < altered.size())
                {
                    // bad idea to put down bracket inside of another group
                    if (altered[j] == '(')
                    {
                        ++unclosedBrackets;
                        ++j;
                    }
                    while (unclosedBrackets > 0 && j < altered.size())
                    {
                        if (altered[j] == '(')
                            ++unclosedBrackets;
                        else if (altered[j] == ')')
                            --unclosedBrackets;
                        ++j;
                    }

                    if (j == altered.size())
                        break;

                    // second phase, look for first valid place to put down ending bracket
                    for (int id = 0; id < operators.size(); ++id)
                        if (operators[id] == altered[j])
                            metOperator = true;
                    if (!metOperator && j < altered.size())
                        ++j;
                }

                // j is right on top of what we need to insert before
                if (autoBracketed)
                    altered.insert(altered.begin() + j, ')');
            }
        }
    }

    // put brackets around individual 'x' and 'y' (helps 'mark' them for the next step)
    for (int i = 0; i < altered.size(); ++i)
        if (altered[i] == 'x' || altered[i] == 'y')
        {
            // possible that this was part of 'exp'
            bool exp = false;
            if (i - 1 >= 0 && i + 1 < altered.size())
               if (altered[i-1] == 'e' && altered[i+1] == 'p')
                   exp = true;

            if (!exp)
            {
                altered.insert(altered.begin() + i + 1, ')');
                altered.insert(altered.begin() + i, '(');
                i += 2;
            }
        }

    // between each pair of:
    // 1) )(
    // 2) )s
    // 3) )c
    // 4) )t
    // 5) )e
    // 6) )a
    // add a * operator
    for (int i = 0; i < altered.size()-1; ++i)
    {
        if(altered[i] == ')')
        {
            char r = altered[i+1];
            if (r == '(' || r == 's' || r == 'c' || r == 't' || r == 'e' || r == 'a')
                altered.insert(altered.begin()+i+1, '*');
        }
    }

    // if a digit (0-9) borders a '(' or ')' or 's', 'c', 't', 'e', add a * operator
    string digits = "0123456789";
    for (int i = 0; i < altered.size(); ++i)
    {
        bool isDigit = false;
        for (int j = 0; j < digits.size(); ++j)
            if (digits[j] == altered[i])
                isDigit = true;
        if (isDigit)
        {
            // e.g 9(
            if (i + 1 < altered.size())
            {
                char r = altered[i+1];
                if(r== '(' || r== 's' || r== 'c'|| r== 't' || r== 'e' || r == 'a')
                     altered.insert(altered.begin() + i + 1, '*');
            }

            // e.g )3
            else if (i - 1 >= 0)
            {
                char r = altered[i-1];
                if(r== '(' || r== 's' || r== 'c'|| r== 't' || r== 'e' || r == 'a')
                    altered.insert(altered.begin() + i, '*');
            }
        }
    }

    return altered;
}


float EquationParser::evaluateEquation(float x, float y)
{
    ae_set("x", x);
    ae_set("y", y);
    return ae_eval(data);
}


string EquationParser::errorCheck(const QVector<QVector<Point> > &gridData)
{
    if (standardized.size() == 0)
        return "No equation was entered";

    // acos and asin are forbidden (lua interpreter does something weird with them...)
    for (int i = 0; i+3 < standardized.size(); ++i)
        if (standardized.substr(i,4) == "acos" || standardized.substr(i,4) == "asin")
            return "acos and asin are forbidden!";

    bool clipped = false;
    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            float val = evaluateEquation(gridData[i][j].x, gridData[i][j].y);

            if (isnan(val))
                return "Undefined values encountered";

            const char* buf = ae_error();

            if (buf != NULL)
            {
                string message(buf);

                // re-state common errors to make it more readable for users
                if (message == "not a number" || message.substr(0, 10) == "attempt to" || message.substr(0,5) == "<eof>")
                    return "Unknown symbols";
                else if (message.substr(0,12) == "')' expected")
                    return "Extra/missing brackets";
                else
                    return message;
            }

            // to be more specific, they will be clipped later on
            else if (val > MAX_POTENTIAL || val < -MAX_POTENTIAL)
                clipped = true;

        }

    if (clipped)
        return "Values were clipped for stability";
    else
        return "";
}
