#include <ctype.h>
#include <stdlib.h>
#include "demos.h"

void stack_init(dstack_t *s) { s->top = 0; }

bool stack_push(dstack_t *s, double v)
{
    if (s->top == sizeof s->data / sizeof s->data[0]) return false;   /* overflow */
    s->data[s->top++] = v;
    return true;
}

bool stack_pop(dstack_t *s, double *v)
{
    if (s->top == 0) return false;                                    /* underflow */
    *v = s->data[--s->top];
    return true;
}

bool brackets_balanced(const char *text)
{
    dstack_t s;
    stack_init(&s);
    for (; *text; text++) {
        char c = *text;
        if (c == '(' || c == '[' || c == '{') {
            if (!stack_push(&s, c)) return false;
        } else if (c == ')' || c == ']' || c == '}') {
            double open;
            char want = c == ')' ? '(' : c == ']' ? '[' : '{';
            if (!stack_pop(&s, &open) || (char)open != want) return false;
        }
    }
    return s.top == 0;
}

bool rpn_eval(const char *expr, double *result)
{
    dstack_t s;
    stack_init(&s);
    while (*expr) {
        if (isspace((unsigned char)*expr)) { expr++; continue; }
        if (isdigit((unsigned char)*expr) || (*expr == '.' )) {
            char *end;
            if (!stack_push(&s, strtod(expr, &end))) return false;
            expr = end;
            continue;
        }
        double b, a;
        if (!stack_pop(&s, &b) || !stack_pop(&s, &a)) return false;
        switch (*expr) {
        case '+': a += b; break;
        case '-': a -= b; break;
        case '*': a *= b; break;
        case '/': if (b == 0) return false; a /= b; break;
        default: return false;
        }
        stack_push(&s, a);
        expr++;
    }
    return s.top == 1 && stack_pop(&s, result);
}
