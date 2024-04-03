#include <concepts>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

class Character;
class PhysicalItem;
class Weapon;
class Potion;
class Spell;

template <typename CurClass>
concept PhysicalDerived = std::is_base_of<CurClass, PhysicalItem>::value;

class Character {
 private:
  int healthPoints;
  std::string name;

 protected:
  void obtainItemSideEffect(const PhysicalItem&);
  void loseItemSideEffect(const PhysicalItem&);
  friend std::ostream& operator<<(std::ostream& out, const Character&);

 public:
  int getHP() const;
  std::string getName() const;
  void takeDamage(int);
  void heal(int);
};
int Character::getHP() const {
  return healthPoints;
}
std::string Character::getName() const {
  return name;
}
void Character::takeDamage(int damage) {
  healthPoints -= damage;
}
void Character::heal(int healVolume) {
  healthPoints += healVolume;
}

class PhysicalItem {
 private:
  bool isUsableOnce;
  Character owner;
  std::string name;

 protected:
  Character getOwner;
  void useCondition(const Character&, const Character&);
  void giveDamageTo(Character&, int);
  void giveHealTo(Character&, int);
  void afterUse();
  virtual void useLogic(const Character&, const Character&) = 0;
  friend std::ostream& operator<<(std::ostream&, const PhysicalItem&);

 public:
  PhysicalItem();
  PhysicalItem(const Character&, const std::string&);
  void use(const Character&, const Character&);
  std::string getName() const;
  virtual void setup() = 0;
};
PhysicalItem::PhysicalItem() : isUsableOnce(false), owner(), name(nullptr) {}
PhysicalItem::PhysicalItem(const Character& ch, const std::string& name) : isUsableOnce(false), owner(ch), name(name) {}
std::string PhysicalItem::getName() const {
  return name;
}
void PhysicalItem::giveDamageTo(Character &target, int damage) {
  target.takeDamage(damage);
}
void PhysicalItem::giveHealTo(Character &target, int healVolume) {
  target.heal(healVolume);
}

class Weapon : public PhysicalItem {
 private:
  int damage;
  virtual void useLogic(const Character&, const Character&) override;

 public:
  int getDamage();
  void setup() override;
  friend std::ostream& operator<<(std::ostream& out, const Weapon& weapon);
};
int Weapon::getDamage() {
  return damage;
}
std::ostream& operator<<(std::ostream& out, const Weapon& weapon) {
    out << weapon.getName() << ":" << weapon.damage;
};

class Potion : public PhysicalItem {
 private:
  int healValue;
  virtual void useLogic(const Character&, const Character&) override;

 public:
  int getHealValue();
  void setup() override;
};
int Potion::getHealValue() {
  return healValue;
}

class Spell : public PhysicalItem {
 private:
  std::vector<Character> allowedTargets;
  void useLogic(const Character&, const Character&) override;

 public:
  size_t getNumAllowedTargets();
  void setup() override;
};
size_t Spell::getNumAllowedTargets() {
  return allowedTargets.size();
}

template <PhysicalDerived T>
class Container {
 protected:
  std::map<std::string, T> elements;
 public:
  virtual void add(T);
  void remove(T) = 0;
  void remove(std::string) = 0;
  bool find(T) = 0;
  T find(std::string) = 0;
};
template <PhysicalDerived T>
void Container<T>::add(T item) {
  std::string itemName = item.getName();
  elements[itemName] = item;
}
template <PhysicalDerived T>
void Container<T>::remove(T item) {
  if (elements.size() == 0 || !find(item))
    throw std::runtime_error("Error caught");
  elements.erase(item.getName());
}
template <PhysicalDerived T>
void Container<T>::remove(std::string name) {
  if (elements.size() == 0 || !find(name))
    throw std::runtime_error("Error caught");
  elements.erase(name);
}
template <PhysicalDerived T>
bool Container<T>::find(T item) {
  return elements.contains(item.getName());
}
template <PhysicalDerived T>
T Container<T>::find(std::string name) {
  if (auto searched = elements.find(name); searched != elements.end())
    return searched->second;
  return NULL;
}

template <PhysicalDerived T>
class ContainerWithMaxCapacity : public Container<T> {
 private:
  int maxCapacity;

 public:
  void add(T) override;
  void show(std::ofstream& out);
};
template <PhysicalDerived T>
void ContainerWithMaxCapacity<T>::show(std::ofstream& out) {
  bool first = true;
  for (T element : Container<T>::elements) {
    if (first) {
      out << element;
      first = false;
    } else {
      out << ' ' << element;
    }
  }
  out << '\n';
}
template <PhysicalDerived T>
void ContainerWithMaxCapacity<T>::add(T item) {
  if (Container<T>::elements.size() == maxCapacity)
    throw std::runtime_error("Error caught");
  Container<T>::add(item);
}

int main() {
}