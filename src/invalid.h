/*
 *	Copyright (c) 2019-2022 Zhao YunShan(93850592@qq.com)
 *	All rights reserved.
 */
#ifndef __INVALID_H__
#define __INVALID_H__

#define INVALID (-1)
#define TIMEOUT (-2)

template <class _CLS>
bool valid(const _CLS &cls)
{
	return cls && cls->valid();
}
template <class _CLS>
bool check_valid(_CLS &cls)
{
	if (cls)
	{
		if (cls->valid())
		{
			return true;
		}
		else
		{
			cls = nullptr;
		}
	}
	return false;
}
#endif
