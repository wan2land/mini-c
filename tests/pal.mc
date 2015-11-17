/*
palindrome
*/

void main()
{
    int org, rev; // org: original, rev: reverse
    int i, j;

    read(org);
    if (org < 0) org = (-1) * org;
    i = org;
    rev = 0;
    while (i != 0) {
        j = i % 10;
        rev = rev * 10 + j;
        i /= 10;
    }
    if (rev == org) write(org);
}
