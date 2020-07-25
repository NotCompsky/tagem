Platform | CI Status
---------|----------
OSX      | [![OSX Build Status](http://badges.herokuapp.com/travis/NotCompsky/tagem?env=BADGE=osx&label=build&branch=master)](https://travis-ci.org/NotCompsky/tagem)
Linux    | [![Linux Build Status](http://badges.herokuapp.com/travis/NotCompsky/tagem?env=BADGE=linux&label=build&branch=master)](https://travis-ci.org/NotCompsky/tagem) [![CircleCI](https://circleci.com/gh/NotCompsky/tagem.svg?style=shield)](https://circleci.com/gh/NotCompsky/tagem)

[![Lines of Code](https://tokei.rs/b1/github/NotCompsky/tagem)](/NotCompsky/tagem/graphs/code-frequency)

[![Docker Image Size](https://img.shields.io/docker/image-size/notcompsky/tagem?label=Docker%20image%20size)](https://hub.docker.com/repository/docker/notcompsky/tagem/tags)
[![Docker Pulls](https://img.shields.io/docker/pulls/notcompsky/tagem)](https://hub.docker.com/repository/docker/notcompsky/tagem)
[![Docker Stars](https://img.shields.io/docker/stars/notcompsky/tagem)](https://hub.docker.com/repository/docker/notcompsky/tagem)

[![Latest Release](https://img.shields.io/github/v/release/NotCompsky/tagem)](/NotCompsky/tagem/releases)
[![Commit Activity](https://img.shields.io/github/commit-activity/w/NotCompsky/tagem)](/NotCompsky/tagem/graphs/commit-activity)

[![License](https://img.shields.io/github/license/NotCompsky/tagem)](LICENSE)
[![Contributers](https://img.shields.io/github/contributors/NotCompsky/tagem)](/NotCompsky/tagem/graphs/contributors)

[![Discord Chat](https://img.shields.io/discord/736649679575580814)](https://discord.gg/DnD7RJA)


# Description

Programs to rapidly categorise and access files, based on assignable attributes such as (heirarchical) tags, named variables, file sizes, hashes, and audio duration.

# Status

In active development.

# Demonstration

A neutered version of this app is hosted [here](https://notcompsky.github.io/tagem-eg/). GitHub does not allow it to be interactive, so most features are disabled - it is basically just a demonstration of the front-end.

A sample of features in the demo:
* The ability to create and view playlists of 'eras' - [e.g.](https://notcompsky.github.io/tagem-eg/#F27240@61.156-71.18095,27240@142.25289-152.22987,27240@179.79853-181.03303,27240@9.92681-40.53999) and [e.g.](https://notcompsky.github.io/tagem-eg/#F1716907@40.18624-62.19732,1716907@508.84786-590.06463,1716905@1413.42456-1479.68176,1717041@24.01881-36.59333,1716865@18.66585-26.29306,1716865@562.62463-709.87243,1705870@4725.33544-4805.94384,1705547@1313.98913-1397.71997,1705815@3253.74096-3333.7163,1716874@579.61828-648.75299,1706072@2119.89111-2256.44628,1705917@2001.07617-2067.96728,1717395@1440.27014-1589.06665,1716948@57.99343-70.49639,1716948@640.38897-730.95611,1716910@1710.13073-1827.7821) and [e.g.](https://notcompsky.github.io/tagem-eg/#F1705345@0-7.34816,1705345@1491.77014-1552.95642,1705351@5.31989-11.7427,1705347@1012.92541-1068.25573,1705347@3019.88085-3087.77734,1705347@3966.50854-3998.92333,1705866@1107.79541-1176.36669,1705866@1745.14575-1813.09997,1705866@2472.19604-2549.5144,1705870@24.32478-92.60926,1705870@2585.84594-2641.48193,1705870@4725.33544-4805.94384,1705547@1313.98913-1397.71997,1705547@2585.45458-2663.77319,1705547@4101.60791-4173.92968,1705470@1943.51049-2002.19677,1705823@24.97947-90.0026,1705832@1375.32324-1536.51953,1705832@3034.65454-3097.44873,1705832@4545.26953-4602.98974,1705846@20.1248-78.50012,1705846@2446.10449-2511.80273,1705846@4625.38525-4694.35742,1705815@1692.83166-1756.73364,1705815@3253.74096-3333.7163,1705815@4719.40087-4822.58154,1705809@1806.38195-1874.09887,1705809@3264.85498-3320.38647,1705809@4831.96435-4884.958,1705860@1758.8861-1822.54785,1705860@3275.333-3332.60791,1705860@4593.94824-4676.33837,1705860@4594.28173-4689.40722,1705878@27.21296-95.50437,1705878@4268.15527-4356.72509,1705891@1689.41638-1758.81506,1705891@3263.03686-3336.74682,1705891@4415.93408-4477.18701,1705412@15.94564-24.24893,1705412@839.64813-896.78051,1705412@1535.432-1619.69934,1705412@2463.92456-2523.91943,1705424@701.73681-764.88629,1705424@1401.32873-1483.94763,1705424@2895.98388-2960.04321,1705398@1012.56713-1095.80151,1705398@1935.06652-1999.94238,1716874@579.61828-648.75299,1705640@1415.02917-1468.69812,1705640@2956.57495-3030.27978,1705640@4385.35888-4456.58007,1706072@2119.89111-2256.44628,1705815@4719.3999-4822.29638,1719652@1322.78466-1372.9884,1719652@1479.03601-1519.01354,1719652@2198.69848-2210.94311,1719652@70.03448-89.14099,1705897@12.64718-73.04966,1705897@1742.54699-1805.734,1705897@4166.44873-4255.32226,1705917@2001.07617-2067.96728,1705917@3655.1582-3757.28491,1705914@895.75866-977.6336,1705909@10.68156-80.87898,1705909@2731.58203-2794.85644,1716818@0-6.82277,1716817@0-10.37127,1716817@3627.72338-3703.10571,1716812@0-8.23822,1716811@0-9.47052,1716810@0-7.93049,1716810@17.89222-25.24168,1716810@711.82012-807.62402,1716809@0-6.8232,1705355@0-7.51299,1705358@0-7.25687,1725941@0-8.27504,1725941@1807.9176-1851.80444,1716822@0-10.66821,1716822@1091.70922-1173.36694,1716822@2665.48144-2738.7622,1716813@0-9.80866,1716813@953.33502-1086.00659,1716813@1917.53857-2075.53613,1716808@0-10.2472,1716808@1188.61364-1275.64941,1716808@2528.53149-2604.33496,1716808@3245.61108-3349.78881,1716804@0-8.31033,1716804@1464.35217-1539.05932,1716804@2456.20385-2520.13916,1716799@0-7.24102,1717382@79.9637-84.10654,1717395@26.19703-36.92356,1717395@1440.27014-1589.06665,1717395@2501.8811-2583.22387,1717395@3628.19555-3716.23193,1717438@1311.00561-1371.21911,1717438@2860.66748-2938.08032,1726344@0-8.90829)


One such disabled (in the demo) feature is `qry`, which the below screenshot pertains to.

![Query3](https://user-images.githubusercontent.com/30552567/86843555-a000e380-c09e-11ea-9a0d-5a5e4ae38261.png)

# Installation

## Server

See [INSTALL.md](INSTALL.md).

## Scripts

You'll probably want to add the [scripts](scripts/) directory to your `PATH` environmental variable, or perhaps just copy the scripts to `/usr/local/bin`.

The [Reddit userscript](browser-extensions/userscripts/reddit.js) can be added the usual way you add userscripts.

# Development

## Back End

See [COMPILING.md](COMPILING.md), [CONTRIBUTING.md](CONTRIBUTING.md), [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md), and [ROADMAP.md](ROADMAP.md).

## Front End

See [CONTRIBUTING.md](CONTRIBUTING.md), [FRONTEND.md](FRONTEND.md), and [ROADMAP.md](ROADMAP.md).

# Background

If you feel like there aren't enough blogs on the internet, [here's another](https://gist.github.com/NotCompsky/f1ab63fa2f191b156b9187b111449d20). It's a look at how this project evolved from some unlikely decisions, as I'm personally interested in how [the Butterfly Effect](https://en.wikipedia.org/wiki/Butterfly_effect) occurs in software development.
