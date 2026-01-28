#include <stdio.h>
#include <string.h>

struct Person {
    char name[50];
    int age;
};

int main() {
    struct Person p;
    strcpy(p.name, "Alice");
    p.age = 25;

    printf("Name: %s\n", p.name);
    printf("Age: %d\n", p.age);

    return 0;
}
