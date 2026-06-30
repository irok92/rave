package tree_sitter_rave_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_rave "github.com/tree-sitter/tree-sitter-rave/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_rave.Language())
	if language == nil {
		t.Errorf("Error loading Rave grammar")
	}
}
