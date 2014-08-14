#ifndef PUERTO_H
#define PUERTO_H


class Lock;
class Condition;

class Puerto {
public:
  Puerto();
  ~Puerto();
  void Send(int message);
  void Receive(int *message);
private:
  bool *flag;
  int *message;
  char **lockname;
  Lock **l;
  Condition *c;
};

#endif
