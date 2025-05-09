#include <iostream>
#include <vector>
#include "interpreter.cpp"   // or wherever you have your interpreter code

int main() {
    using namespace std;

    // --- Helper: wrap a std::vector<double> into an ArrayInfo ---
    auto makeDenseMatrix = [&](const string& name, int n, const vector<double>& elems) {
        ArrayInfo m;
        m.shape = { n, n };
        m.data = elems;
        arrays[name] = m;
    };

    // 2×2 example: det([[1,2],[3,4]]) = 1*4 - 2*3 = -2
    makeDenseMatrix("A", 2, { 1, 2,
                              3, 4 });
    {
        ArgsInfo arg = makeArgsInfo(__LINE__, "A", false, "", 0.0);
        IdentifierReturn r = evaluateFunction("DET", { arg });
        cout << "det(A) = " << r.d << "  (expected -2)\n";
    }

    // 3×3 example: det([[6,1,1],[4,-2,5],[2,8,7]]) = 6(-2*7 - 5*8) - 1(4*7 - 5*2) + 1(4*8 - (-2)*2)
    // = 6(-14 - 40) - (28 - 10) + (32 + 4) = 6*(-54) - 18 + 36 = -324 - 18 + 36 = -306
    makeDenseMatrix("B", 3, {
        6,  1,  1,
        4, -2,  5,
        2,  8,  7
    });
    {
        ArgsInfo arg = makeArgsInfo(__LINE__, "B", false, "", 0.0);
        IdentifierReturn r = evaluateFunction("DET", { arg });
        cout << "det(B) = " << r.d << "  (expected -306)\n";
    }

    // Identity matrix: det(I₅) == 1
    vector<double> I5(25, 0.0);
    for (int i = 0; i < 5; ++i) I5[i*5 + i] = 1.0;
    makeDenseMatrix("I5", 5, I5);
    {
        ArgsInfo arg = makeArgsInfo(__LINE__, "I5", false, "", 0.0);
        IdentifierReturn r = evaluateFunction("DET", { arg });
        cout << "det(I5) = " << r.d << "  (expected 1)\n";
    }

    return 0;
}
