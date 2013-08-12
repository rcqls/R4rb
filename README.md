# R for Ruby

This is an attempt to embed the R language in ruby.

## Install

Clone this git and

	rake package
	[sudo] gem install pkg/R4rb-???.gem

## Example (very basic)
```{.ruby execute="false"}
require 'R4rb'
Array.initR
"rnorm(10)".to_R
R4rb << "1:3"
R4rb < "1:3"

# A multilines call 
R4rb << <<REND
data(iris)
a <- iris[[1]]
REND

R4rb < "a"
```

Notice that only R vector are converted to ruby object!
