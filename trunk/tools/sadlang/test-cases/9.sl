code atFileStart """
int g;
"""

code atFileEnd """
void f(void)
{
    g++;
}
"""

code atMainLoopStart """
    PRINTF("hello world\n");
"""

code atMainLoopEnd """
    PRINTF("goodbye world\n");
"""
