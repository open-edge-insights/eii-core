/*
Copyright (c) 2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package eiiconfigmgr

import (
	"fmt"
	"errors"
)

type Expr interface {
	IsExpr()
}

type ValType int

const (
    String ValType =  iota
    Int
    Array
    Json
    Boolean
    Float32
)

func (valType ValType) String() string {
    return [...]string{"String", "Int", "Array", "Json", "Boolean", "Float32"}[valType]
}

type integer int

func (i integer) IsExpr() {
}

func (i integer) getInteger() {}

type boolean bool
func (b boolean) IsExpr() {}

type float float32
func (f float) IsExpr() {}

type str string
func (s str) IsExpr() {}

type object map[string]interface{}

func (obj object) IsExpr() {}

type array []interface{}

func (arr array) IsExpr() {}

func eval (e Expr) (r Expr) {
    switch exp := e.(type) {
		case integer: r = exp
		case boolean: r = exp
		case float: r = exp
		case str: r = exp
		case object: r = exp
		case array: r = exp
    }
    return
}

// strcut to define config type and values
type ConfigValue struct{
	Type ValType
	Value Expr
}

// GetInteger gets integer value from the value received from GetInterfaceValue
//
// Returns:
// 1. integer value : integer
//    returns integer value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetInteger() (integer, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == Int) {
		return x.(integer), nil
	}
	return -1, errors.New("Value in not int type")
}

// GetFloat gets float value from the value received from GetInterfaceValue
//
// Returns:
// 1. float value : float
//    returns float value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetFloat() (float, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == Float32) {
		return x.(float), nil
	}
	return -1, errors.New("Value in not float type")
}

// GetString gets string value from the value received from GetInterfaceValue
//
// Returns:
// 1. string value : string
//    returns string value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetString() (string, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == String) {
		return fmt.Sprintf("%v", x), nil
	}
	return "", errors.New("Value in not string type")
}

// GetBool gets boolean value from the value received from GetInterfaceValue
//
// Returns:
// 1. bool value : bool
//    returns bool value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetBool() (boolean, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == Boolean) {
		return x.(boolean), nil
	}
	return false, errors.New("Value in not bool type")
}

// GetJson gets json value from the value received from GetInterfaceValue
//
// Returns:
// 1. json value : object
//    returns json value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetJson() (object, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == Json) {
		return x.(object), nil
	}
	return nil, errors.New("Value in not json type")
}

// GetArray gets array value from the value received from GetInterfaceValue
//
// Returns:
// 1. array value : array
//    returns array value
// 2. error
//    Error on failure,  nil on success
func (cfg ConfigValue) GetArray() (array, error) {
	var x interface{} = cfg.Value
	if(cfg.Type == Array) {
		return  x.(array), nil
	}
	return nil, errors.New("Value in not Array type")
}
