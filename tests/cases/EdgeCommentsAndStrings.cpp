# 1 "/tmp/ctags-capture/sketches/EdgeCommentsAndStrings/EdgeCommentsAndStrings.ino"
// Comments and string literals containing braces, parentheses and
// comment markers. arduino-cli preprocesses with -CC, so ctags sees
// the comments.

/* void notAFunction() { } */

// void alsoNotAFunction() { }

const char *braces = "{ ( } ) // /* */";
const char quote = '"';
const char brace = '{';
const char *raw = R"(void rawString() { return; })";

int commentInSignature(int a /* first */, int b /* second */) {
  return a + b;
}

int commentAfterParen(int a) // trailing comment
{
  return a;
}

int commentBeforeBrace(int a) /* block comment */ {
  return a;
}

/*
 * Multi-line comment before a function.
 */
int afterBlockComment(int a) {
  // Unbalanced in a comment: {{{
  return a;
}

int stringWithBrace(int a) {
  const char *s = "}";
  (void) s;
  return a;
}

int charWithBrace(int a) {
  char c = '}';
  (void) c;
  return a;
}

int escapedQuote(int a) {
  const char *s = "\"}\"";
  (void) s;
  return a;
}

int stringEndsWithEscapedQuote(int a) {
  char temp[27] = "AT + CMGS = \"";
  (void) temp;
  return a;
}

int charEscapedQuote(int a) {
  char c = '\"';
  (void) c;
  return a;
}

int afterStrings(int a) {
  return a;
}

void setup() {
  commentInSignature(1, 2);
  commentAfterParen(1);
  commentBeforeBrace(1);
  afterBlockComment(1);
  stringWithBrace(1);
  charWithBrace(1);
  escapedQuote(1);
  stringEndsWithEscapedQuote(1);
  charEscapedQuote(1);
  afterStrings(1);
}

void loop() {
}
