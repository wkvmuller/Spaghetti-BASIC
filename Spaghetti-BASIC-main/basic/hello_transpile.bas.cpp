#include <iostream>
using namespace std;

int main() {
  L10:
  double A = 5;
  L20:
  std::cout << "INPUT B: "; std::cin >> B;
  L30:
  if (A > B) goto L50;
  L40:
  std::cout << "B << is << greater << or << equal", << B << std::endl;
  L45:
  goto L60;
  L50:
  std::cout << "A << is << greater", << A << std::endl;
  L60:
  do {
  L70:
  std::cout << "Looping..." << std::endl;
  L80:
  } while (true);
  L90:
  return 0;
  return 0;
}
