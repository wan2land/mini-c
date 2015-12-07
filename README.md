Mini C
===

## Trouble Shooting

만약 `bison`에서 `minic.y: conflicts: 1 shift/reduce` 에러가 발생한다면?

 - `bison -d minic.y -v` 를 입력하면 output이 출력 됨. 거기서 원인을 찾자.

## 변경된 부분 AST

```
    Nonterminal: CALL
        Terminal: write
        Nonterminal: ACTUAL_PARAM
            Terminal: i
```

이거 교과서에는 

```
    Nonterminal: CALL
        Terminal: write
        Terminal: i
```

아마 이렇게 나와있을 거임.