#pragma once
/*
  Copyright 2022-2023 Stefan Grimm

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "state.h"
#include "lokilight.h"

namespace tsmlib {

namespace impl {

template<
  class Event,
  class To_true,
  class To_false,
  class From,
  class CreationPolicy,
  class Guard,
  class Action,
  bool IsExitingTransition>
struct ChoiceTransitionBase {

  enum { E = false };
  enum { X = IsExitingTransition };

  typedef Event EventType;
  typedef To_false ToType;
  typedef From FromType;
  typedef CreationPolicy CreationPolicyType;
  typedef typename CreationPolicy::ObjectType StateType;

  ChoiceTransitionBase() {
    // Choice without guard does not make sense; the choice is either to go the the state To_true or To_false.
    CompileTimeError< !is_same<Guard, NoGuard>().value >();
    // Choice transition
    CompileTimeError< !is_same<To_true, EmptyState<typename CreationPolicy::ObjectType>>().value >();
    CompileTimeError< !is_same<From, EmptyState<typename CreationPolicy::ObjectType>>().value >();
  }

  DispatchResult<StateType> dispatch(StateType* activeState) {

    typedef typename To_true::CreatorType ToFactory;
    typedef typename From::CreatorType FromFactory;

    Action().perform(activeState);

    if (Guard().eval(activeState)) {
      return execute< To_true >(activeState);
    } else {
      return execute< To_false >(activeState);
    }
  }

private:
  template<class To>
  DispatchResult<StateType> execute(StateType* activeState) {

    typedef typename To::CreatorType ToFactory;
    typedef typename From::CreatorType FromFactory;

    // Self transition
    if (is_same<To, From>().value) {
      static_cast<To*>(activeState)->template _doit<EventType>();
      return DispatchResult<StateType>(true, activeState);
    }

    if (X) {
      return DispatchResult<StateType>(false, activeState);
    }

    static_cast<From*>(activeState)->template _exit<EventType>();
    FromFactory::destroy(static_cast<From*>(activeState));

    To* toState = ToFactory::create();
    toState->template _entry<EventType>();
    if (is_base_of<BasicState<To, StateType>, To>::value) {
      toState->template _doit<EventType>();
    }
    return DispatchResult<StateType>(true, toState);
  }
};

}

template<class Event, class To_true, class To_false, class From, class CreationPolicy, class Guard, class Action>
using ChoiceTransition = impl::ChoiceTransitionBase<Event, To_true, To_false, From, CreationPolicy, Guard, Action, false>;

template<class Event, class To_true, class To_false, class From, class CreationPolicy, class Guard, class Action>
using ChoiceExitTransition = impl::ChoiceTransitionBase<Event, To_true, To_false, From, CreationPolicy, Guard, Action, true>;

}
