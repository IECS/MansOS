/**
 * Visual Blocks Language
 *
 * Copyright 2012 Google Inc.
 * http://code.google.com/p/blockly/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview Generating Seal for logic blocks.
 * @author fraser@google.com (Neil Fraser)
 */
'use strict';

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Seal.logic_compare = function() {
  // Comparison operator.
  var mode = this.getTitleValue('OP');
  var operator = Blockly.Seal.logic_compare.OPERATORS[mode];
  var order = Blockly.Seal.ORDER_RELATIONAL;
  var argument0 = Blockly.Seal.valueToCode(this, 'A', order) || '0';
  var argument1 = Blockly.Seal.valueToCode(this, 'B', order) || '0';
  var code = argument0 + ' ' + operator + ' ' + argument1;
  return [code, order];
};

Blockly.Seal.logic_compare.OPERATORS = {
  EQ: '==',
  NEQ: '!=',
  LT: '<',
  LTE: '<=',
  GT: '>',
  GTE: '>='
};

Blockly.Seal.logic_operation = function() {
  // Operations 'and', 'or'.
  var operator = (this.getTitleValue('OP') == 'AND') ? 'and' : 'or';
  var order = (operator == 'and') ? Blockly.Seal.ORDER_LOGICAL_AND :
      Blockly.Seal.ORDER_LOGICAL_OR;
  var argument0 = Blockly.Seal.valueToCode(this, 'A', order) || 'False';
  var argument1 = Blockly.Seal.valueToCode(this, 'B', order) || 'False';
  var code = argument0 + ' ' + operator + ' ' + argument1;
  return [code, order];
};

Blockly.Seal.logic_negate = function() {
  // Negation.
  var argument0 = Blockly.Seal.valueToCode(this, 'BOOL',
      Blockly.Seal.ORDER_LOGICAL_NOT) || 'False';
  var code = 'not ' + argument0;
  return [code, Blockly.Seal.ORDER_LOGICAL_NOT];
};

Blockly.Seal.logic_boolean = function() {
  // Boolean values true and false.
  var code = (this.getTitleValue('BOOL') == 'TRUE') ? 'True' : 'False';
  return [code, Blockly.Seal.ORDER_ATOMIC];
};
