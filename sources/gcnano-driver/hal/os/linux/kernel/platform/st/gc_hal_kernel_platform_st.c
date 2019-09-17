/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2018 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2018 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/slab.h>

#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel_platform.h"

struct st_priv {
    /* Clock management.*/
    struct clk  *clk_3d_ahb;
    struct clk  *clk_3d_axi;
    struct clk  *clk_3d_clk2x;
};

static struct st_priv *stpriv;

static int
_AllocPriv(IN gcsPLATFORM * Platform)
{
    struct st_priv *priv;

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        gckOS_Print("galcore platform st: cannot allocate memory.\n");
        return -ENOMEM;
    }

    stpriv = priv;
    return 0;
}

static int
_FreePriv(IN gcsPLATFORM * Platform)
{
    struct st_priv *priv = stpriv;

    if (priv)
        kfree(priv);

    stpriv = NULL;
    return 0;
}

gceSTATUS
_AdjustParam(IN gcsPLATFORM * Platform, OUT gcsMODULE_PARAMETERS *Args)
{
    struct platform_device* pdev = Platform->device;
    struct device *dev = &pdev->dev;
    struct resource *res, contig_res;
    struct device_node *np;
    int irq;

    if (of_device_is_compatible(dev->of_node, "vivante,gc")) {
        /* Register base address */
        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!res) {
            gckOS_Print("galcore platform st: missing reg base address.\n");
            return gcvSTATUS_OUT_OF_RESOURCES;
        }
        Args->registerMemBase = res->start;
        Args->registerMemSize = resource_size(res);

        np = of_parse_phandle(dev->of_node, "contiguous-area", 0);
        if (np) {
            if (of_address_to_resource(np, 0, &contig_res)) {
                gckOS_Print("galcore platform st: no contiguous-area resource.\n");
                return gcvSTATUS_OUT_OF_RESOURCES;
            }
            Args->contiguousBase = contig_res.start;
            Args->contiguousSize = resource_size(&contig_res);
        }

        /* Core interrupt line */
        irq = platform_get_irq(pdev, 0);
        if (irq < 0) {
            gckOS_Print("galcore platform st: missing core interrupt line.\n");
            return gcvSTATUS_NOT_SUPPORTED;
        }
        Args->irqLine = irq;

    } else {
        /* Register base address */
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "iobase_3d");

        if (res) {
            Args->registerMemBase = res->start;
            Args->registerMemSize = resource_size(res);
        }

        /* 3D pipeline IRQ */
        irq = platform_get_irq_byname(pdev, "irq_3d");
        if (irq)
            Args->irqLine = irq;

        /* Contiguous area */
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
                                           "contig_baseaddr");

        if (res) {
                Args->contiguousBase = res->start;
                Args->contiguousSize = res->end - res->start + 1;
        }
    }

    return gcvSTATUS_OK;
}

static const struct of_device_id gcnano_of_match[] = {
    { .compatible = "st,gcnano" },
    { .compatible = "st,gcnano-ultra" },
    { /* end node */ }
};
MODULE_DEVICE_TABLE(of, gcnano_of_match);

gceSTATUS
_GetPower(IN gcsPLATFORM * Platform)
{
    struct device *dev = &Platform->device->dev;
    struct st_priv *priv = stpriv;
    gceSTATUS ret_val = gcvSTATUS_OK;

    /*Initialize the clock structure*/
    priv->clk_3d_ahb = devm_clk_get(dev, "clk_3d_ahb");
    if (IS_ERR(priv->clk_3d_ahb)) {
        gckOS_Print("galcore platform st: unable to get clk_3d_ahb clock.\n");
        priv->clk_3d_ahb = NULL;
        ret_val = gcvSTATUS_CLOCK_ERROR;
    }

    if (of_device_is_compatible(dev->of_node, "st,gcnano-ultra")) {
        priv->clk_3d_axi = devm_clk_get(dev, "clk_3d_axi");
        if (IS_ERR(priv->clk_3d_axi)) {
            gckOS_Print("galcore platform st: unable to get clk_3d_axi clock.\n");
            priv->clk_3d_axi = NULL;
            ret_val = gcvSTATUS_CLOCK_ERROR;
        }
    }

    priv->clk_3d_clk2x = devm_clk_get(dev, "clk_3d_clk2x");
    if (IS_ERR(priv->clk_3d_clk2x)) {
        gckOS_Print("galcore platform st: unable to get clk_3d_clk2x clock.\n");
        priv->clk_3d_clk2x = NULL;
        ret_val = gcvSTATUS_CLOCK_ERROR;
    }

    return ret_val;
}

gceSTATUS
_PutPower(IN gcsPLATFORM * Platform)
{
    struct st_priv *priv = stpriv;
        struct device *dev = &Platform->device->dev;
    /*Disable clock*/
    if (priv->clk_3d_ahb) {
        devm_clk_put(dev,priv->clk_3d_ahb);
        priv->clk_3d_ahb = NULL;
    }

    if (priv->clk_3d_axi) {
        devm_clk_put(dev,priv->clk_3d_axi);
        priv->clk_3d_axi = NULL;
    }

    if (priv->clk_3d_clk2x) {
        devm_clk_put(dev,priv->clk_3d_clk2x);
        priv->clk_3d_clk2x = NULL;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
_SetClock(IN gcsPLATFORM * Platform, IN gceCORE GPU, IN gctBOOL Enable)
{
    struct st_priv* priv = stpriv;

    if (Enable) {
        if (priv->clk_3d_ahb)
            if (clk_prepare_enable(priv->clk_3d_ahb))
                gckOS_Print("galcore platform st: failed to clock clk_3d_ahb\n");

        if (priv->clk_3d_axi)
            if (clk_prepare_enable(priv->clk_3d_axi))
                gckOS_Print("galcore platform st: failed to clock clk_3d_axi\n");

        if (priv->clk_3d_clk2x)
            if (clk_prepare_enable(priv->clk_3d_clk2x))
                gckOS_Print("galcore platform st: failed to clock clk_3d_clk2x\n");
    } else {
        if (priv->clk_3d_ahb)
            clk_disable_unprepare(priv->clk_3d_ahb);

        if (priv->clk_3d_axi)
            clk_disable_unprepare(priv->clk_3d_axi);

        if (priv->clk_3d_clk2x)
            clk_disable_unprepare(priv->clk_3d_clk2x);
    }

    return gcvSTATUS_OK;
}

static struct soc_platform_ops st_ops =
{
    .adjustParam   = _AdjustParam,
    .getPower      = _GetPower,
    .putPower      = _PutPower,
    .setClock      = _SetClock,
};

static struct soc_platform st_platform =
{
    .name = __FILE__,
    .ops  = &st_ops,
};

int soc_platform_init(struct platform_driver *pdrv,
            struct soc_platform **platform)
{
    int ret;
    pdrv->driver.of_match_table = gcnano_of_match;

    ret = _AllocPriv(&st_platform);

    *platform = &st_platform;
    return ret;
}

int soc_platform_terminate(struct soc_platform *platform)
{
    _FreePriv(platform);
    return 0;
}

