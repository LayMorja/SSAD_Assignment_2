#include <concepts>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Classes declaration for correct handling
class Character;
class PhysicalItem;
class Weapon;
class Potion;
class Spell;

template <typename CurClass>
concept PhysicalDerived = std::is_base_of<PhysicalItem, CurClass>::value;

template <typename CurClass>
concept CharacterDerived = std::is_base_of<Character, CurClass>::value;

// --- CHARACTER DECLARATION ---
class Character {
private:
  int healthPoints;
  std::string name;

protected:
  virtual void obtainItemSideEffect(const PhysicalItem &) = 0;
  virtual void loseItemSideEffect(const PhysicalItem &) = 0;
  friend std::ostream &operator<<(std::ostream &, const Character &);

public:
  int getHP() const;
  std::string getName() const;
  void takeDamage(int);
  void heal(int);
};
// --- CHARACTER DEFINITION ---
int Character::getHP() const { return healthPoints; }
std::string Character::getName() const { return name; }
void Character::takeDamage(int damage) { healthPoints -= damage; }
void Character::heal(int healVolume) { healthPoints += healVolume; }
std::ostream &operator<<(std::ostream &out, const Character &character) {
  out << character.name << ":" << character.healthPoints;
  return out;
}

// --- ITEM DECLARATION ---
class PhysicalItem {
private:
  bool isUsableOnce;
  std::unique_ptr<Character> owner;
  std::string name;

protected:
  std::unique_ptr<Character> getOwner;
  void useCondition(const Character &, const Character &);
  void giveDamageTo(Character &, int);
  void giveHealTo(Character &, int);
  void afterUse();
  virtual void useLogic(const Character &, const Character &) = 0;
  friend std::ostream &operator<<(std::ostream &, const PhysicalItem &);

public:
  PhysicalItem();
  PhysicalItem(std::unique_ptr<Character>, const std::string &);
  void use(const Character &, const Character &);
  std::string getName() const;
  virtual void setup() = 0;
};
// --- ITEM DEFINITION ---
PhysicalItem::PhysicalItem() : isUsableOnce(false), owner(), name(nullptr) {}
PhysicalItem::PhysicalItem(std::unique_ptr<Character> ch,
                           const std::string &name)
    : isUsableOnce(false), owner(std::move(ch)), name(name) {}
std::string PhysicalItem::getName() const { return name; }
void PhysicalItem::giveDamageTo(Character &target, int damage) {
  target.takeDamage(damage);
}
void PhysicalItem::giveHealTo(Character &target, int healVolume) {
  target.heal(healVolume);
}

// --- WEAPON DECLARATION ---
class Weapon : public PhysicalItem {
private:
  int damage;
  virtual void useLogic(const Character &, const Character &) override;
  friend std::ostream &operator<<(std::ostream &out, const Weapon &weapon);

public:
  Weapon() = delete;
  explicit Weapon(int);
  int getDamage();
  void setup() override;
};
// --- WEAPON DEFINITION ---
Weapon::Weapon(int damage) : damage(damage) {}
int Weapon::getDamage() { return damage; }
std::ostream &operator<<(std::ostream &out, const Weapon &weapon) {
  out << weapon.getName() << ":" << weapon.damage;
  return out;
};
// --- POTION DECLARATION ---
class Potion : public PhysicalItem {
private:
  int healValue;
  virtual void useLogic(const Character &, const Character &) override;
  friend std::ostream &operator<<(std::ostream &out, const Potion &potion);

public:
  int getHealValue();
  void setup() override;
};
// --- POTION DEFINITION ---
int Potion::getHealValue() { return healValue; }
std::ostream &operator<<(std::ostream &out, const Potion &potion) {
  out << potion.getName() << ":" << potion.healValue;
  return out;
};
// --- SPELL DECLARATION ---
class Spell : public PhysicalItem {
private:
  std::vector<Character> allowedTargets;
  void useLogic(const Character &, const Character &) override;
  friend std::ostream &operator<<(std::ostream &, const Spell &);

public:
  size_t getNumAllowedTargets() const;
  void setup() override;
};
// --- SPELL DEFINITION ---
size_t Spell::getNumAllowedTargets() const { return allowedTargets.size(); }
std::ostream &operator<<(std::ostream &out, const Spell &spell) {
  out << spell.getName() << ':' << spell.getNumAllowedTargets();
  return out;
};

template <typename T> class Container {};

// --- CONTAINER WITH CONCEPT DECLARATION ---
template <PhysicalDerived T> class Container<T> {
protected:
  std::map<std::string, T> elements;

public:
  virtual void add(T);
  virtual void remove(T) ;
  virtual void remove(std::string);
  virtual bool find(T);
  virtual T find(std::string);
};
// --- CONTAINER DEFINITION ---
template <PhysicalDerived T> void Container<T>::add(T item) {
  std::string itemName = item.getName();
  elements[itemName] = item;
}
template <PhysicalDerived T> void Container<T>::remove(T item) {
  if (elements.size() == 0 || !find(item))
    throw std::runtime_error("Error caught");
  elements.erase(item.getName());
}
template <PhysicalDerived T> void Container<T>::remove(std::string name) {
  if (elements.size() == 0 || !find(name))
    throw std::runtime_error("Error caught");
  elements.erase(name);
}
template <PhysicalDerived T> bool Container<T>::find(T item) {
  return elements.contains(item.getName());
}
template <PhysicalDerived T> T Container<T>::find(std::string name) {
  if (auto searched = elements.find(name); searched != elements.end())
    return searched->second;
  return NULL;
}

// --- CONTAINER CAPACITY DECLARATION ---
template <PhysicalDerived T>
class ContainerWithMaxCapacity : public Container<T> {
private:
  int maxCapacity;

public:
  void add(T) override;
  void show(std::ofstream &out);
};
// --- CONTAINER CAPACITY DEFINITION ---
template <PhysicalDerived T>
void ContainerWithMaxCapacity<T>::show(std::ofstream &out) {
  bool first = true;
  for (auto iter = Container<T>::elements.begin(); iter != Container<T>::elements.end(); ++iter) {
    if (first) {
      out << iter->second;
      first = false;
    } else {
      out << ' ' << iter->second;
    }
  }
  out << '\n';
}
template <PhysicalDerived T> void ContainerWithMaxCapacity<T>::add(T item) {
  if (Container<T>::elements.size() == maxCapacity)
    throw std::runtime_error("Error caught");
  Container<T>::add(item);
}
// Typed container representations
using Arsenal = ContainerWithMaxCapacity<Weapon>;
using MedicalBag = ContainerWithMaxCapacity<Potion>;
using SpellBook = ContainerWithMaxCapacity<Spell>;

class WeaponUser : public Character {
protected:
  Arsenal arsenal;
public:
  void attack(std::unique_ptr<Character>, const std::string&);
  void showWeapons(std::ofstream& out);
};
void WeaponUser::attack(std::unique_ptr<Character> target, const std::string& weapon) {
  int damage = arsenal.find(weapon).getDamage();
  target->takeDamage(damage);
}
void WeaponUser::showWeapons(std::ofstream& out) {
  arsenal.show(out);
}

class FantasyStory {
private:
  std::ifstream in;  // reading stream "input.txt"
  std::ofstream out; // writing stream "output.txt"
  int actions;       // number of input commands to validate

public:
  FantasyStory(); // take commands, start the game
  FantasyStory(const FantasyStory &) = delete;
  FantasyStory &operator=(const FantasyStory &) = delete;

  void startStoryTelling(); // process input line by line
  void processTheCommand(const std::string &);
};
FantasyStory::FantasyStory() {
  int actions_count;
  in >> actions_count;
  startStoryTelling();
}
void FantasyStory::startStoryTelling() {
  std::string current_command;
  for (int i = 0; i != actions; ++i) {
    getline(in, current_command);

    try {
      processTheCommand(current_command);
    } catch (const std::runtime_error &err) {
      out << err.what() << '\n';
    }
  }
}
void FantasyStory::processTheCommand(const std::string &command) {}

int main() { FantasyStory start; }
