#pragma once
// Written by Stefan Grimm, 2023.
// Released into the public domain.

/*
  The aim is to write unit tests that work with MSTest and ArduinoUnit.

  ArduinoUnit does not have a test class, so BEGIN and END are simply empty for ArduinoUnit.
  The setup function in the Arduino sketch is the unit test initialize method. INIT is not executed.

  TEST is the method name and does not have a definition like INIT. This way it is possible to debug in Visual Studio.

  Usage:
  BEGIN(CalculatorTest)

  INIT(Initialize, { reset(); })

  TEST(
    first_value_is_1,
    value_2_is_added,
    result_is_3)
  {
    Calculator c;
    c.set(1);
    c.add(2);
    EQ(3, c.result());
  }

  END

  Output:
  GIVEN_first_value_is_1_WHEN_value_2_is_added_THEN_result_is_3

  Spec:
  
  Begin test scenario.
  name: class name
  BEGIN(name)
  
  End test scenario.
  END
  
  Initialize test with the definition.
  name: method name
  definition: method body
  INIT(name, definition)
  
  Test name followed by the method body.
  given, when, then: steps
  TEST(given, when, then)
  
  Assert are equal.
  EQ(a, b)

  Assert are not equal.
  NEQ(a, b)

  Is true.
  TRUE(a)
  
  Is false.
  FALSE(a)

  Assert is null.
  N(a)

  Assert is not null.
  NN(a)
  */

#ifdef MS_CPP_UNITTESTFRAMEWORK

#define BEGIN(name) TEST_CLASS(name) {
#define END };
#define INIT(name, definition) TEST_METHOD_INITIALIZE(name) definition
#define TEST(given, when, then) TEST_METHOD(GIVEN_##given##_WHEN_##when##_THEN_##then)
#define EQ(a, b) Microsoft::VisualStudio::CppUnitTestFramework::Assert::AreEqual(a, b)
#define NEQ(a, b) Microsoft::VisualStudio::CppUnitTestFramework::Assert::AreNotEqual(a, b)
#define TRUE(a) Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsTrue(a)
#define FALSE(a) Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsFalse(a)
#define N(a) Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsNull(a)
#define NN(a) Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsNotNull(a)

#else defined(ARDUINO_UNIT_TESTS)

#define BEGIN(name)
#define END
#define INIT(name, definition)
#define TEST(given, when, then) test(GIVEN_##given##_WHEN_##when##_THEN_##then)
#define EQ(a, b) assertEqual(a, b)
#define NEQ(a, b) assertNotEqual(a, b)
#define TRUE(a) assertTrue(a)
#define FALSE(a) assertFalse(a)
#define N(a) assertEqual(true, (a == nullptr))
#define NN(a) assertEqual(true, (a != nullptr))

#endif
