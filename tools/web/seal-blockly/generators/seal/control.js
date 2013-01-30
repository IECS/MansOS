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
 * @fileoverview Generating Python for control blocks.
 * @author fraser@google.com (Neil Fraser)
 */
'use strict';

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Seal.controls_if = function () {
	// while/elsewhile/else condition.
	var n = 0;
	var argument = Blockly.Seal.valueToCode(this, 'IF' + n,
		Blockly.Seal.ORDER_NONE) || 'False';
	var branch = Blockly.Seal.statementToCode(this, 'DO' + n) || ' \n';
	var code = 'When ' + argument + ':\n' + branch;
	for (n = 1; n <= this.elseifCount_; n++) {
		argument = Blockly.Seal.valueToCode(this, 'IF' + n,
			Blockly.Seal.ORDER_NONE) || 'False';
		branch = Blockly.Seal.statementToCode(this, 'DO' + n) || ' \n';
		code += 'else when ' + argument + ':\n' + branch;
	}
	if (this.elseCount_) {
		branch = Blockly.Seal.statementToCode(this, 'ELSE') || ' \n';
		code += 'else:\n' + branch;
	}
	return code + "end\n";
};