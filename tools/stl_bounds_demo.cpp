#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <iterator>

using namespace std;

namespace
{
  typedef vector<int> vec;
  vec v;

  void bounds(int k)
  {
    vec::iterator lb = lower_bound(v.begin(), v.end(), k);
    cout << "lower_bound " << k << " is " << (lb != v.end() ? *lb : -1)
         << ", index " << distance(v.begin(), lb) << '\n';
    vec::iterator ub = upper_bound(v.begin(), v.end(), k);
    cout << "upper_bound " << k << " is " << (ub != v.end() ? *ub : -1)
         << ", index " << distance(v.begin(), ub) << '\n';
  }
}

int main()
{
  for (int i = 1; i <= 5; i += 2)
  {
    v.push_back(i);
    cout << i << " ";
    if (i == 3)
    {
      v.push_back(i);
      cout << i << " ";
      v.push_back(i);
      cout << i << " ";
    }
  }
  cout << '\n';
  for (int j = 0; j <= 6; ++j)
    bounds(j);
 return 0;
}

/* Output:
1 3 3 3 5
lower_bound 0 is 1, index 0
upper_bound 0 is 1, index 0
lower_bound 1 is 1, index 0
upper_bound 1 is 3, index 1
lower_bound 2 is 3, index 1
upper_bound 2 is 3, index 1
lower_bound 3 is 3, index 1
upper_bound 3 is 5, index 4
lower_bound 4 is 5, index 4
upper_bound 4 is 5, index 4
lower_bound 5 is 5, index 4
upper_bound 5 is -1, index 5
lower_bound 6 is -1, index 5
upper_bound 6 is -1, index 5
*/
