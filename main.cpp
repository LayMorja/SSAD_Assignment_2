#include <concepts>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
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

// --- CHARACTER DECLARATION ---
class Character {
private:
  int healthPoints;
  std::string name;

protected:
/*
  virtual void obtainItemSideEffect(const PhysicalItem &) = 0;
  virtual void loseItemSideEffect(const PhysicalItem &) = 0;
*/
  friend std::ostream &operator<<(std::ostream &, const Character &);

public:
  Character(int, const std::string&);
  int getHP() const;
  std::string getName() const;
  void takeDamage(int);
  void heal(int);
  virtual ~Character() = default;
};
// --- CHARACTER DEFINITION ---
Character::Character(int healthPoints, const std::string& name) : healthPoints(healthPoints), name(name) {}
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
  std::shared_ptr<Character> owner;
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
  PhysicalItem(bool, std::shared_ptr<Character>, const std::string &);
  void use(const Character &, const Character &);
  std::string getName() const;
  virtual void setup() = 0;
};
// --- ITEM DEFINITION ---
PhysicalItem::PhysicalItem() : isUsableOnce(false), owner(), name(nullptr) {}
PhysicalItem::PhysicalItem(bool usage, std::shared_ptr<Character> ch,
                           const std::string &name)
    : isUsableOnce(usage), owner(std::move(ch)), name(name) {}
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
Weapon::Weapon(int damage) : isUsableOnce(false), damage(damage) {}
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

/*template <typename T> class Container {};*/

// --- CONTAINER WITH CONCEPT DECLARATION ---
template <PhysicalDerived T> class Container {
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

class WeaponUser : virtual public Character {
protected:
  Arsenal arsenal;
public:
  WeaponUser(int, const std::string&);
  void attack(std::unique_ptr<Character>, const std::string&);
  void showWeapons(std::ofstream& out);
};
WeaponUser::WeaponUser(int healthPoints, const std::string &name) : Character(healthPoints, name)  {}
void WeaponUser::attack(std::unique_ptr<Character> target, const std::string& weapon) {
  int damage = arsenal.find(weapon).getDamage();
  target->takeDamage(damage);
}
void WeaponUser::showWeapons(std::ofstream& out) {
  arsenal.show(out);
}
class PotionUser : virtual public Character {
protected:
  MedicalBag medicalBag;
public:
  PotionUser(int, const std::string&);
  void drink(std::shared_ptr<Character>, const std::string&);
  void showPotions(std::ofstream&);
};
PotionUser::PotionUser(int healthPoints, const std::string &name) : Character(healthPoints, name)  {}
void PotionUser::drink(std::shared_ptr<Character> target, const std::string &potion) {
  int healValue = medicalBag.find(potion).getHealValue();
  target->heal(healValue);
}
void PotionUser::showPotions(std::ofstream& out) {
  medicalBag.show(out);
}
class SpellUser : virtual public Character {
protected:
  SpellBook spellBook;
public:
  SpellUser(int, const std::string&);
  void cast(std::shared_ptr<Character>, const std::string&);
  void showSpells(std::ofstream&);
};
SpellUser::SpellUser(int healthPoints, const std::string &name) : Character(healthPoints, name)  {}
void SpellUser::cast(std::shared_ptr<Character> target, const std::string &potion) {
  Spell spell = spellBook.find(potion);
  spell.use(*this, target);
}
void SpellUser::showSpells(std::ofstream& out) {
  spellBook.show(out);
}

class Fighter : public WeaponUser, public PotionUser {
public:
  Fighter(int, const std::string &);
};
Fighter::Fighter(int healthPoints, const std::string& name) : Character(healthPoints, name), WeaponUser(healthPoints, name), PotionUser(healthPoints, name) {}
class Archer : public WeaponUser, public PotionUser, public SpellUser {
  Archer(int, const std::string &);
};
Archer::Archer(int healthPoints, const std::string& name) : Character(healthPoints, name), WeaponUser(healthPoints, name), PotionUser(healthPoints, name), SpellUser(healthPoints, name) {}
class Wizard : public PotionUser, public SpellUser {
  Wizard(int, const std::string &);
};
Wizard::Wizard(int healthPoints, const std::string& name) : Character(healthPoints, name), PotionUser(healthPoints, name), SpellUser(healthPoints, name) {}

class FantasyStory {
private:
  std::ifstream in;  // reading stream "input.txt"
  std::ofstream out; // writing stream "output.txt"
  int actions;       // number of input commands to validate
  std::map<std::string, std::shared_ptr<Character>> characters;

public:
  FantasyStory(); // take commands, start the game
  FantasyStory(const FantasyStory &) = delete;
  FantasyStory &operator=(const FantasyStory &) = delete;

  void startStoryTelling(); // process input line by line
  void processAction(const std::string &);
  void createCharacter(std::istringstream&);
  void createItem(std::istringstream&);
};
FantasyStory::FantasyStory() {
  in >> actions;
  startStoryTelling();
}
void FantasyStory::startStoryTelling() {
  std::string current_command;
  for (int i = 0; i != actions; ++i) {
    getline(in, current_command);

    try {
      processAction(current_command);
    } catch (const std::runtime_error &err) {
      out << err.what() << '\n';
    }
  }
}
void FantasyStory::processAction(const std::string& actionLine) {
  std::istringstream action(actionLine);
  std::string currentAction;
  action >> currentAction;

  if (currentAction == "Create") {
    std::string creation_type;
    in >> creation_type;
    if (creation_type == "character") {
      createCharacter(action);
    } else {
      createItem(action);
    }
  } else if (currentAction == "Show") {
  } else if (currentAction == "Dialogue") {
  } else {
  }
}
void FantasyStory::createCharacter(std::istringstream &actionLine) {
  std::string type, name;
  int healthPoints;
  actionLine >> type >> name >> healthPoints;

  if (type == "fighter") {
    std::shared_ptr<Character> fighter = std::make_shared<Fighter>(healthPoints, name);
    characters.insert(std::make_pair(name, fighter));
  } else if (type == "wizard") {

  } else {

  }
  characters.insert(std::make_pair(name, ))
}

int main() { FantasyStory start; }
