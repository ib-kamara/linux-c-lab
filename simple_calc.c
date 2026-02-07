#include <stdio.h>
#include <stdlib.h>

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    return a / b;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: ./simple_calc <number1> <operator> <number2>\n");
        return 1;
    }

    double num1 = atof(argv[1]);
    char op = argv[2][0];
    double num2 = atof(argv[3]);

    double answer;

    if (op == '+') {
        answer = add(num1, num2);
        printf("Result: %.2f\n", answer);
    }
    else if (op == '-') {
        answer = subtract(num1, num2);
        printf("Result: %.2f\n", answer);
    }
    else if (op == '*') {
        answer = multiply(num1, num2);
        printf("Result: %.2f\n", answer);
    }
    else if (op == '/') {
        if (num2 == 0) {
            printf("Error: Division by zero is not allowed.\n");
            return 1;
        }
        answer = divide(num1, num2);
        printf("Result: %.2f\n", answer);
    }
    else {
        printf("Error: Unsupported operator '%c'. Use +, -, * or /.\n", op);
        return 1;
    }

    return 0;
}
