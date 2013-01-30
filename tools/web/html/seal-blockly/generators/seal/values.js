'use strict';

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Seal.seal_number = function () {
	// Numeric value.
	var code = window.parseFloat(this.getTitleValue('NUM'));
	return [code, Blockly.Seal.ORDER_UNARY_SIGN];
};

Blockly.Seal.seal_sensors = function () {
	return [this.getTitleValue('VALUE')];
};

Blockly.Seal.seal_struct = function () {
	return [this.getTitleValue('NAME') + "." + Blockly.Seal.valueToCode(this, 'ARGS')];
};
