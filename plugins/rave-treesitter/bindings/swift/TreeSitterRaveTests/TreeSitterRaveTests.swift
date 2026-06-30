import XCTest
import SwiftTreeSitter
import TreeSitterRave

final class TreeSitterRaveTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_rave())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Rave grammar")
    }
}
