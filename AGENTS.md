# Filesystem structure

Ignore files with names starting with __ (double underscore). They're temporary work or notes, not intended to compile.

# Review

When doing code review, check all of the following points:
* Understand the context - Read the PR description, related issues, and understand what problem is being solved
* Write a summary what the change really does
* Check correctness - Verify the logic is sound, edge cases are handled, and the implementation matches requirements
* Assess code quality - Look for readability, proper naming, appropriate abstractions, and adherence to coding standards
* Security review - Check for potential vulnerabilities, input validation, and secure coding practices
* Search for bugs
* Performance considerations - Identify potential bottlenecks, inefficient algorithms, or resource usage issues
* Test coverage - Ensure adequate tests exist and run them to verify functionality
* Documentation - Check that code comments and documentation are updated as needed, if the functions are non-trivial
  suggest comments to add which will explain what the functions or classes do.
* Check if existing comments around function still correspond to what functions actually do
* Check if existing comments around variables still correctly describe those variables
* Check if changed behavior still makes sense
* Check for English typos
* Propose a better comments if you think some might be misunderstood
* If new utility is added (function, class) check if there's already existing tool in the codebase to avoid duplication.

# Code quality

Remember that using static auto variables in functions might be thread-unsafe if they're not read-only.

If methods are marked as const, it's especially important to keep them thread-safe (eg. for use with std algorithms).
Consider using mutexes or nu::Synchronized and mutable class members to protect thread-safety.

Prefer to use physical quantity types if possible, eg. use si::Length instead of float or double.

Move constructor and move operator= should be marked noexcept if possible.  Some things use `std::move_if_noexcept`. If
your object is not noexcept, they fall back to copying, perhaps unnecessarily.

Make sure to use Rule-of-5.

Never write `~Class() = default` alone unless the intent is to add the `virtual` specifier.

Make sure to have virtual destructor if at least one virtual method is present (virtual destructor can be in the base
class already, so you don't have to add another one in the inherited class).

# Code style

Use tabs of width 4 for indentation.

## Methods

One-line methods should be implemented directly in the class (only in-class methods, not free functions):

    class X
    {
        void
        oneliner()
            { do_something(); }
    }

Free functions (even one-liners) should use this style:

void
func()
{
    ...
}

Multi-line functions should be moved outside the class with exception that when it requires a lot of redundant
`template<>` writing, or `requires` clause that has to be written again at the definition site it's okay to have a
multi-line method directly in the class.

### Arguments

If an argument isn't to be modified, it should be const. It's not necessary to use `const` on function/method
declarations as opposed to definitions.

### Other

* Beware of Argument-Dependent Lookup, put specializations into std in the same header as the class definition.
* boost::format bug with `uint8_t` arguments. Use `static_cast<int>` on `uint8_t` arguments.
  Or better just use `std::format`.
* All const-methods should be read-only-thread-safe.
  Remember when using "mutable" members in objects, or when mutating any other (global or not) state from
  const method. Use `std::atomic<>`, `nu::Synchronized<>` to help.
