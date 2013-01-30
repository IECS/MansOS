'use strict';

Blockly.Seal = Blockly.Generator.get('Seal');

Blockly.Seal.seal_use = function () {
	return 'Use ' + this.getTitleValue('USE') + Blockly.Seal.valueToCode(this, 'ARGS') + ';\n';
};


Blockly.Seal.seal_read = function () {
	return 'Read ' + this.getTitleValue('READ') + Blockly.Seal.valueToCode(this, 'ARGS') + ';\n';
};

Blockly.Seal.seal_output = function () {
	return 'Output ' + this.getTitleValue('OUTPUT') + Blockly.Seal.valueToCode(this, 'ARGS') + ';\n';
};

Blockly.Seal.lists_create_with = function () {
	// Create a list with any number of elements of any type.
	var code = new Array(this.itemCount_);
	for (var n = 0; n < this.itemCount_; n++) {
		code[n] = Blockly.Seal.valueToCode(this, 'ADD' + n,
			Blockly.Seal.ORDER_NONE) || 'None';
	}
	code = 'NetworkRead ' + this.getTitleValue('NAME') + '(' + code.join(', ') + ');\n';
	return [code, Blockly.Seal.ORDER_ATOMIC];
};