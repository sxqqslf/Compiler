FUNCTION power :
PARAM v1
PARAM v2
v3 := #1
LABEL label1 :
IF v2 <= #0 GOTO label3
v3 := v3 * v1
v2 := v2 - #1
GOTO label1
LABEL label3 :
RETURN v3
FUNCTION mod :
PARAM v4
PARAM v5
t6 := v4 / v5
t5 := t6 * v5
t4 := v4 - t5
RETURN t4
FUNCTION getNumDigits :
PARAM v6
v7 := #0
IF v6 >= #0 GOTO label5
t9 := #0 - #1
RETURN t9
LABEL label5 :
LABEL label6 :
IF v6 <= #0 GOTO label8
v6 := v6 / #10
v7 := v7 + #1
GOTO label6
LABEL label8 :
RETURN v7
FUNCTION isNarcissistic :
PARAM v8
ARG v8
t14 := CALL getNumDigits
v9 := t14
v10 := #0
v11 := v8
LABEL label9 :
IF v11 <= #0 GOTO label11
ARG #10
ARG v11
v12 := CALL mod
t18 := v11 - v12
v11 := t18 / #10
ARG v9
ARG v12
t20 := CALL power
v10 := v10 + t20
IF v10 != v8 GOTO label13
RETURN #1
GOTO label14
LABEL label13 :
RETURN #0
LABEL label14 :
GOTO label9
LABEL label11 :
FUNCTION printHexDigit :
PARAM v13
IF v13 >= #10 GOTO label16
WRITE v13
GOTO label17
LABEL label16 :
t24 := #0 - v13
WRITE t24
LABEL label17 :
RETURN #0
FUNCTION printHex :
PARAM v14
DEC v15 16
v16 := #0
LABEL label18 :
IF v16 >= #4 GOTO label20
ARG #16
ARG v14
t28 := CALL mod
t32 := v16 * #4
t30 := &v15 + t32
*t30 := t28
v14 := v14 / #16
v16 := v16 + #1
GOTO label18
LABEL label20 :
v16 := #3
LABEL label21 :
IF v16 < #0 GOTO label23
t37 := v16 * #4
t35 := &v15 + t37
ARG *t35
(null) := CALL printHexDigit
v16 := v16 - #1
GOTO label21
LABEL label23 :
RETURN #0
FUNCTION main :
v17 := #0
v18 := #9400
LABEL label24 :
IF v18 >= #9500 GOTO label26
ARG v18
t42 := CALL isNarcissistic
IF t42 != #1 GOTO label28
ARG v18
(null) := CALL printHex
v17 := v17 + #1
LABEL label28 :
v18 := v18 + #1
GOTO label24
LABEL label26 :
RETURN v17
