// The Term class is modeled after Prolog terms.

package coop.rchain.storage

import scala.collection.mutable.ArrayBuffer

trait TermTree {
  val term: String // entire term, including params for a key

  def isKey: Boolean = { this.isInstanceOf[Key] }
  def isVariable: Boolean = { this.isInstanceOf[Variable] }
  def isConstant: Boolean = { this.isInstanceOf[Constant] }
  def display: Unit
}

class Params(paramsArray: ArrayBuffer[TermTree]) extends TermTree {
  if (paramsArray.length == 0) {
    throw new RChainException("Params: no params to constructor")
  }

  protected[storage] val params = paramsArray
  protected[storage] var _term = "("
  for (i <- 0 until params.length - 1) {
    _term += params(i).term + ","
  }
  _term += params(params.length - 1).term + ")"
  val term = _term

  val length = paramsArray.length

  def display: Unit = { print(term) }
}

trait Atom extends TermTree {}

class Variable(param: String) extends Atom {
  protected[storage] val variable = param.trim

  if (!TermTools.isVariable(variable))
    throw new RChainException(s"Variable(): malformed param: '$param'")

  val term = variable

  def display: Unit = { print("'" + variable + "'") }
}

// constants are numerals or names

class Constant(param: String) extends Atom {
  protected[storage] val constant = param.trim

  if (!TermTools.isConstant(constant))
    throw new RChainException(s"Constant(): malformed param: '$param'")

  val term = constant

  def display: Unit = { print("'" + constant + "'") }
}

object TermTools {
  def createTermTree(term: String): TermTree = {
    // This method is not intended to create Params
    if (isVariable(term)) return new Variable(term)
    if (isConstant(term)) return new Constant(term)
    // KeyLexer requires the first term be a key
    assert(term.indexOf("(") < term.indexOf(")"))
    val lexer = new KeyLexer(term)
    val lexToken = lexer.NextToken
    if (lexToken.token == Token.Key)
      return new Key(term)
    throw new RChainException(
      "createTermTree: not recognized: " + lexToken.tokenStr)
  }

  def isVariable(s: String): Boolean = {
    if (s == null || s.isEmpty)
      throw new RChainException("isVariable argument is null or empty")
    s(0).isUpper
  }
  def isConstant(s: String): Boolean = {
    if (s == null || s.isEmpty)
      throw new RChainException("isConstant argument is null or empty")
    s(0).isLower || s(0).isDigit
  }
}
