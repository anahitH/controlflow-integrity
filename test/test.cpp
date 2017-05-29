#include <stdio.h>
#define PROTECTED __attribute__((annotate("protected")))

void display() {

}

void calc() {

}

void notify() {
}

PROTECTED
void readKey() {
  printf("Super Secret\n");
}

void encrypt() {
  readKey();
}

void decrypt() {
  readKey();
}

void loadUI() {
  display();
}

void operation1() {
  calc();
  notify();
}

void operation2() {
  encrypt();
  decrypt();
}

int main() {
  loadUI();
  operation1();
  operation2();
}
