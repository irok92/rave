/**
 * @file A fullstack programming language to target multiple devices in the same codebase.
 * @author Mindrage <mindrage@live.com>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
  name: "rave",

  rules: {
    // TODO: add the actual grammar rules
    source_file: $ => "hello"
  }
});
