//-*-C++-*-
// Copyright © 2015 Richard Kettlewell.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ACTION_H
#define ACTION_H
/** @file Action.h
 * @brief Concurrent operations
 */

#include <list>
#include <set>

class ActionList;
class EventLoop;

/** @brief One action that may be initiated concurrently */
class Action {
public:
  /** @brief Constructor
   * @param g Concurrency group
   */
  Action(): running(false) {
  }

  /** @brief Destructor */
  virtual ~Action();

  /** @brief Specify a resource that this action uses
   * @param g Concurrency group
   */
  void uses(const std::string &r) {
    resources.push_back(r);
  }

  /** @brief Start the action
   * @param e Event loop
   * @param al Execution context
   */
  virtual void go(EventLoop *e, ActionList *al) = 0;

  /** @brief Called when the action is complete
   * @param e Event loop
   * @param al Execution context
   *
   * The default implementation does nothing.
   */
  virtual void done(EventLoop *e, ActionList *al);

private:
  friend class ActionList;

  /** @brief Resources required by this action */
  std::vector<std::string> resources;

  /** @brief Current state */
  bool running;
};

/** @brief A collection of actions that are excecuted concurrently */
class ActionList {
public:
  /** @brief Constructor
   * @param e Event loop
   */
  ActionList(EventLoop *e): eventloop(e) {
  }

  /** @brief Add an action
   * @param a Action
   */
  void add(Action *a);

  /** @brief Initiate actions
   *
   * Returns when all actions are complete.
   */
  void go();

  /** @brief Called when an action is complete */
  void completed(Action *a);

private:
  /** @brief Event loop */
  EventLoop *eventloop;

  /** @brief List of remaining actions */
  std::list<Action *> actions;

  /** @brief Start any new actions if possible */
  void trigger();

  /** @brief In-use resources */
  std::set<std::string> resources;
};

#endif /* ACTION_H */
