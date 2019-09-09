# N-Squared Input Method

An XIM-based input method for traditional Chinese characters. 

You know the `N` is unspeakable here if you are a Hong Konger. About
`3^2+2^(-j)` for big `j` but `j` cannot be infinite because it is their
trademark. I don't think this is
a patent infringement, as a quick search in the
[Hong Kong Intellectual Property Department database](https://esearch.ipd.gov.hk/nis-pos-view/pt#/quicksearch)
shows that there is no in-force patents about the input method in Hong Kong.

A significant population of Hong Kongers and Macaneses
have learnt to type with this input method in their primary
schools, and the company has been ignoring Linux users for 20+
years despite our requests.

If you are from the company and want me to take down this repo, please tell me
which patent I am infringing, or any legal reasons that I should. I will be
happy to take this down if it is convincing – and please consider releasing a
Linux version and *take your bug report emails seriously*.


# Prerequisites

You need `qt5-devel` and [xcb-imdkit](https://gitlab.com/fcitx/xcb-imdkit)
to be able to compile the program. The latter is not included in most Linux
distributions' package managers, so you will need to install it manually.

*Important: You should use the latest version of `xcb-imdkit` or `nsim` may
crash. This is due to the bug fixed in [this commit](https://gitlab.com/fcitx/xcb-imdkit/commit/4a04ba78c51fba58594e22997bff252590083597). Any versions later than this commit are fine.*

# Compiling and Installing

```
cd src/frontend
qmake
make
```

The above commands will give you a standalone executable called `nsim`. Just put
it into your `$PATH`. The binary does not depend on any data files.


# Usage

Run `nsim` when you start your X session (like how you start your ibus or scim daemons),
then add the following to your `~/.profile`

```
export GTK_IM_MODULE="xim"
export QT_IM_MODULE="xim"
export XMODIFIERS="@im=nsim"
```


